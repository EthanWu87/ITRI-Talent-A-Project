#include "dw1000.h"
#include "bmc81m001.h"
#include "ei_main.h"
#include "ht32f493x5_int.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UART_MASK (DW1000_RX_BUFFER_SIZE - 1U)
#define CMD_MAX 120U
#define UWB_LINE_MAX 160U
#define TX_MAX 256U
#define TX_TIMEOUT_MS 1000U

static volatile uint8_t uwb_ring[DW1000_RX_BUFFER_SIZE];
static volatile uint16_t uwb_w, uwb_r;
static volatile uint32_t uwb_drop;

typedef struct
{
  uint8_t data[TX_MAX];
  volatile uint16_t length;
  volatile uint16_t position;
} dw1000_tx_t;

static dw1000_tx_t uwb_tx;

/* Main context owns the buffer until it is published to the TX interrupt. */
static uint8_t dw1000_tx_bytes(usart_type *port, dw1000_tx_t *tx,
                               const uint8_t *data, size_t length)
{
  if(data == NULL || length == 0U || length > sizeof(tx->data) || tx->position < tx->length)
    return 0U;
  usart_interrupt_enable(port, USART_TDBE_INT, FALSE);
  memcpy(tx->data, data, length);
  tx->position = 0U;
  tx->length = (uint16_t)length;
  __DMB();
  usart_interrupt_enable(port, USART_TDBE_INT, TRUE);
  return 1U;
}

static uint8_t dw1000_tx_start(usart_type *port, dw1000_tx_t *tx, const char *text)
{
  return dw1000_tx_bytes(port, tx, (const uint8_t *)text, strlen(text));
}

void dw1000_tx_irq_handler(usart_type *port)
{
  dw1000_tx_t *tx = &uwb_tx;
  if(usart_interrupt_flag_get(port, USART_TDBE_FLAG) == RESET)
  {
    return;
  }
  if(tx->position < tx->length)
  {
    usart_data_transmit(port, tx->data[tx->position++]);
  }
  if(tx->position >= tx->length)
  {
    usart_interrupt_enable(port, USART_TDBE_INT, FALSE);
  }
}

static uint8_t dw1000_tx_done(usart_type *port, const dw1000_tx_t *tx)
{
  return tx->position == tx->length && usart_flag_get(port, USART_TDC_FLAG) != RESET;
}

static void dw1000_tx_cancel(usart_type *port, dw1000_tx_t *tx)
{
  usart_interrupt_enable(port, USART_TDBE_INT, FALSE);
  tx->position = 0U;
  tx->length = 0U;
}

void dw1000_receive_byte(uint8_t data)
{
  uint16_t next = (uint16_t)((uwb_w + 1U) & UART_MASK);
  if(next != uwb_r)
  {
    uwb_ring[uwb_w] = data;
    uwb_w = next;
  }
  else
  {
    uwb_drop++;
  }
}

static int dw1000_ring_get(void)
{
  uint8_t data;
  if(uwb_r == uwb_w)
  {
    return -1;
  }
  data = uwb_ring[uwb_r];
  uwb_r = (uint16_t)((uwb_r + 1U) & UART_MASK);
  return (int)data;
}

static void dw1000_ring_clear(void) { uwb_r = uwb_w; }

static void dw1000_usart_configuration(void)
{
  gpio_init_type gpio_init_struct;

  /* Enable UART8 and GPIO clocks. Default mapping: PE1 TX / PE0 RX. */
  crm_periph_clock_enable(DW1000_USART_CLK, TRUE);
  crm_periph_clock_enable(DW1000_USART_TX_GPIO_CLK, TRUE);
  crm_periph_clock_enable(DW1000_USART_RX_GPIO_CLK, TRUE);

  /* Configure UART8 TX pin. */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = DW1000_USART_TX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(DW1000_USART_TX_GPIO_PORT, &gpio_init_struct);

  /* Configure UART8 RX pin, matching the MR60BHA2 configuration. */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = DW1000_USART_RX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(DW1000_USART_RX_GPIO_PORT, &gpio_init_struct);

  /* Configure UART8 NVIC interrupt. */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(DW1000_USART_IRQn, 0, 0);

  /* Configure UART8 parameters. */
  usart_init(DW1000_USART_PORT, DW1000_BAUD_RATE, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_parity_selection_config(DW1000_USART_PORT, USART_PARITY_NONE);
  usart_transmitter_enable(DW1000_USART_PORT, TRUE);
  usart_receiver_enable(DW1000_USART_PORT, TRUE);

  /* Enable UART8 receive interrupt. */
  usart_interrupt_enable(DW1000_USART_PORT, USART_RDBF_INT, TRUE);
  usart_enable(DW1000_USART_PORT, TRUE);
}

static uint8_t measuring = 0;
static unsigned active_seq = 0;
static uint8_t have_first = 0, have_second = 0;
static char measurement_kind[8];
static uint8_t have_third = 0;
static uint8_t dw1000_result_line(const char * response, const char * word)
{
  const char * p = response;
  size_t n = strlen(word);
  while (* p)
  {
    const char * start = p;
    while (* p && * p != '\r' && * p != '\n') p++;
    if (*p != '\0' && (size_t) (p - start) == n && strncmp(start, word, n) == 0) return 1;
    while (* p == '\r' || * p == '\n') p++;
  }
  return 0;
}
/* UWB AT commands and measurement lines arrive through DW1000_USART_PORT ISR ring. */
static char uwb_line[UWB_LINE_MAX];
static uint16_t uwb_line_n = 0;
static uint8_t uwb_line_overflow;
static char uwb_at[320];
static uint16_t uwb_at_n = 0;
static uint8_t uwb_collect_at = 0;
static char uwb_role[24] = "UNKNOWN";
static int uwb_anchor_id = -1;
static char uwb_last_failure[40] = "NONE";
#define UWB_RETRIES       3U
#define UWB_QUIET_MS      220U
#define UWB_RESET_MS      1700U
/* Anchor calibration: discard initial reports, then take a median per link.
 * Keep the existing PC protocol: exactly one DIST message per pair / START.
 * Avoid TCP send inside UART parsing: send from main loop when ready.
 */
#define CAL_WARMUP_SAMPLES  2U
#define CAL_MEDIAN_SAMPLES  5U  /* Increase to 9 after verifying timing */
typedef struct
{
  float values[CAL_MEDIAN_SAMPLES];
  uint8_t warmup;
  uint8_t count;
  uint8_t pending;
  uint8_t sent;
  float median;
}
calibration_samples_t;
static calibration_samples_t cal_ab, cal_ac, cal_bc;
static void dw1000_cal_reset(calibration_samples_t * c)
{
  memset(c, 0, sizeof(* c));
}

static float dw1000_cal_median(const float * src, uint8_t count)
{
  float a[CAL_MEDIAN_SAMPLES];
  uint8_t i, j;
  float t;
  for (i = 0U; i < count;++i) a[i] = src[i];
  for (i = 1U; i < count;++i)
  {
    t = a[i];
    j = i;
    while (j > 0U && a[j -1U] > t)
    {
      a[j] = a[j -1U];
--j;
    }
    a[j] = t;
  }
  return a[count / 2U];
  /* CAL_MEDIAN_SAMPLES is odd */
}

static void dw1000_cal_add(calibration_samples_t * c, float value)
{
  if (c->pending || c->sent) return;
  if (c->warmup < CAL_WARMUP_SAMPLES)
  {
++c->warmup;
    return;
  }
  if (c->count < CAL_MEDIAN_SAMPLES) c->values[c->count++] = value;
  if (c->count == CAL_MEDIAN_SAMPLES)
  {
    c->median = dw1000_cal_median(c->values, c->count);
    c->pending = 1U;
  }
}

static void dw1000_cal_emit(calibration_samples_t * c, const char * pair)
{
  char value[42];
  if (!c->pending || c->sent || !measuring) return;
  snprintf(value, sizeof(value), "%s,%.3f", pair, (double) c->median);
  printf("[CAL FILTER] %s median=%.3f, warmup=%u, samples=%u\r\n", pair, (double) c->median, (unsigned) c->warmup, (unsigned) c->count);
  if(flow_wifi_send_control("DIST", active_seq, value))
  {
    c->pending = 0U;
    c->sent = 1U;
  }
}

static void dw1000_cal_send_pending(void)
{
  if (!measuring || strcmp(uwb_role, "TAG") != 0) return;
  if (strcmp(measurement_kind, "ABAC") == 0)
  {
    dw1000_cal_emit(& cal_ab, "AB");
    dw1000_cal_emit(& cal_ac, "AC");
  }
  else if (strcmp(measurement_kind, "BC") == 0)
  {
    dw1000_cal_emit(& cal_bc, "BC");
  }
}

static void dw1000_handle_uwb_line(const char * line)
{
  char temp[UWB_LINE_MAX], * token;
  unsigned anchor;
  float dist;
  if (!measuring || strcmp(uwb_role, "TAG") != 0) return;
  strncpy(temp, line, sizeof(temp) -1U);
  temp[sizeof(temp) -1U] = 0;
  token = strtok(temp, ",");
  while (token)
  {
    if (sscanf(token, "an%u:%fm", & anchor, & dist) == 2 && dist > 0.0f && dist < 100.0f)
    {
      char value[42];
      if (strcmp(measurement_kind, "ABAC") == 0)
      {
        if (anchor == 1U) dw1000_cal_add(& cal_ab, dist);
        else if (anchor == 0U) dw1000_cal_add(& cal_ac, dist);
      }
      else if (strcmp(measurement_kind, "BC") == 0)
      {
        if (anchor == 0U) dw1000_cal_add(& cal_bc, dist);
      }
      else if (strcmp(measurement_kind, "TRACK") == 0)
      {
        /* Preserve the existing Tag TRACK behavior for compatibility. */
        const char * pair = NULL;
        uint8_t * received = NULL;
        if (anchor == 2U)
        {
          pair = "TA";
          received = & have_first;
        }
        else if (anchor == 1U)
        {
          pair = "TB";
          received = & have_second;
        }
        else if (anchor == 0U)
        {
          pair = "TC";
          received = & have_third;
        }
        if (pair && !* received)
        {
          snprintf(value, sizeof(value), "%s,%.3f", pair, (double) dist);
          if(flow_wifi_send_control("DIST", active_seq, value))
          {
            *received = 1U;
          }
        }
      }
    }
    token = strtok(NULL, ",");
  }
}

static void dw1000_uwb_byte(uint8_t b)
{
  if (uwb_collect_at && uwb_at_n < sizeof(uwb_at) -1U)
  {
    uwb_at[uwb_at_n++] = (char) b;
    uwb_at[uwb_at_n] = 0;
  }
  if (b == '\r') return;
  if (b == '\n')
  {
    if (uwb_line_n && !uwb_line_overflow)
    {
      uwb_line[uwb_line_n] = 0;
      dw1000_handle_uwb_line(uwb_line);
    }
    uwb_line_n = 0;
    uwb_line_overflow = 0;
    return;
  }
  if (uwb_line_overflow) return;
  if (uwb_line_n < UWB_LINE_MAX -1U) uwb_line[uwb_line_n++] = (char) b;
  else
  {
    uwb_line_n = 0;
    uwb_line_overflow = 1U;
  }
}

static void dw1000_poll_uwb(void)
{
  int b;
  uint16_t budget = DW1000_RX_BUFFER_SIZE;
  while(budget-- != 0U && (b = dw1000_ring_get()) >= 0)
  {
    dw1000_uwb_byte((uint8_t)b);
  }
}

typedef enum
{
  ROLE_IDLE, ROLE_QUIET, ROLE_TX, ROLE_REPLY, ROLE_RETRY, ROLE_REBOOT
} dw1000_role_state_t;

typedef enum
{
  ROLE_CACHED, ROLE_PRECHECK, ROLE_SET, ROLE_RESET, ROLE_VERIFY, ROLE_INTERVAL, ROLE_DISPLAY
} dw1000_role_step_t;

static dw1000_role_state_t role_state;
static dw1000_role_step_t role_step;
static uint32_t role_since, role_timeout;
static uint8_t role_attempt, role_attempts;
static char role_command[64], role_target[24];
static int role_anchor;
static unsigned role_seq;

static void dw1000_role_abort(void)
{
  if(role_state != ROLE_IDLE)
  {
    strcpy(uwb_role, "UNKNOWN");
    uwb_anchor_id = -1;
  }
  role_state = ROLE_IDLE;
  uwb_collect_at = 0U;
  /* An in-flight UART8 command finishes in the ISR; its result is ignored. */
}

static void dw1000_role_step_start(dw1000_role_step_t step, uint32_t now)
{
  role_step = step;
  role_attempt = 1U;
  role_attempts = UWB_RETRIES;
  role_timeout = 2500U;
  switch(step)
  {
    case ROLE_CACHED:
      role_attempts = 2U;
      role_timeout = 1500U;
      strcpy(role_command, "AT\r\n");
      break;
    case ROLE_PRECHECK:
    case ROLE_VERIFY:
      role_timeout = 1500U;
      strcpy(role_command, "AT\r\n");
      break;
    case ROLE_SET:
      if(strcmp(role_target, "TAG") == 0)
      {
        strcpy(role_command, "AT+anchor_tag=0\r\n");
      }
      else
      {
        snprintf(role_command, sizeof(role_command), "AT+anchor_tag=1,%d\r\n", role_anchor);
      }
      break;
    case ROLE_RESET:
      role_attempts = 2U;
      role_timeout = 3500U;
      strcpy(role_command, "AT+RST\r\n");
      break;
    case ROLE_INTERVAL: strcpy(role_command, "AT+interval=5\r\n"); break;
    case ROLE_DISPLAY: strcpy(role_command, "AT+switchdis=1\r\n"); break;
  }
  role_state = ROLE_QUIET;
  role_since = now;
  uwb_collect_at = 0U;
}

static uint8_t dw1000_role_begin(const char *target, int anchor, unsigned seq, uint32_t now)
{
  uint8_t cached;
  if(strcmp(target, "TAG") != 0 && strcmp(target, "ANCHOR") != 0)
  {
    strcpy(uwb_last_failure, "BAD_ROLE");
    return 0U;
  }
  if(strcmp(target, "ANCHOR") == 0 && (anchor < 0 || anchor > 3))
  {
    strcpy(uwb_last_failure, "BAD_ANCHOR_ID");
    return 0U;
  }
  measuring = 0U;
  cached = strcmp(uwb_role, target) == 0 && (strcmp(target, "TAG") == 0 || uwb_anchor_id == anchor);
  strcpy(role_target, target);
  role_anchor = anchor;
  role_seq = seq;
  if(!cached)
  {
    strcpy(uwb_role, "UNKNOWN");
    uwb_anchor_id = -1;
  }
  dw1000_role_step_start(cached ? ROLE_CACHED : ROLE_PRECHECK, now);
  return 1U;
}

static void dw1000_role_complete(void)
{
  strcpy(uwb_role, role_target);
  uwb_anchor_id = strcmp(role_target, "ANCHOR") == 0 ? role_anchor : -1;
  strcpy(uwb_last_failure, "NONE");
  role_state = ROLE_IDLE;
  flow_wifi_send_control("READY", role_seq, role_target);
}

static void dw1000_role_failed(uint32_t now)
{
  static const char * const failures[] = {
    "CACHED_ROLE", "AT_PRECHECK", "SET_ROLE", "RESET_COMMAND",
    "RESET_RECOVERY", "TAG_INTERVAL", "TAG_SWITCHDIS"
  };
  uwb_collect_at = 0U;
  dw1000_tx_cancel(DW1000_USART_PORT, &uwb_tx);
  if(role_attempt < role_attempts)
  {
    role_attempt++;
    role_since = now;
    role_state = ROLE_RETRY;
    return;
  }
  strcpy(uwb_role, "UNKNOWN");
  uwb_anchor_id = -1;
  if(role_step == ROLE_CACHED)
  {
    dw1000_role_step_start(ROLE_PRECHECK, now);
    return;
  }
  strcpy(uwb_last_failure, failures[role_step]);
  role_state = ROLE_IDLE;
  flow_wifi_send_control("ERROR", role_seq, "UWB_ROLE_FAILED");
}

static void dw1000_role_tick(uint32_t now)
{
  if(role_state == ROLE_IDLE)
  {
    return;
  }
  if(role_state == ROLE_REBOOT)
  {
    if((uint32_t)(now - role_since) >= UWB_RESET_MS)
    {
      dw1000_role_step_start(ROLE_VERIFY, now);
    }
    return;
  }
  if(role_state == ROLE_RETRY)
  {
    if((uint32_t)(now - role_since) >= (role_step == ROLE_VERIFY ? 500U : 450U))
    {
      role_state = ROLE_QUIET;
      role_since = now;
    }
    return;
  }
  if(role_state == ROLE_QUIET)
  {
    if((uint32_t)(now - role_since) < UWB_QUIET_MS)
    {
      return;
    }
    dw1000_ring_clear();
    uwb_line_n = 0U;
    uwb_line_overflow = 0U;
    uwb_at_n = 0U;
    uwb_at[0] = '\0';
    uwb_collect_at = 1U;
    if(!dw1000_tx_start(DW1000_USART_PORT, &uwb_tx, role_command))
    {
      dw1000_role_failed(now);
      return;
    }
    role_state = ROLE_TX;
    role_since = now;
    return;
  }
  if(role_state == ROLE_TX)
  {
    if(dw1000_tx_done(DW1000_USART_PORT, &uwb_tx))
    {
      role_state = ROLE_REPLY;
      role_since = now;
    }
    else
    {
      if((uint32_t)(now - role_since) >= TX_TIMEOUT_MS)
      {
        dw1000_role_failed(now);
      }
      return;
    }
  }
  if(role_state == ROLE_REPLY)
  {
    if(dw1000_result_line(uwb_at, "ERROR"))
    {
      dw1000_role_failed(now);
      return;
    }
    if(!dw1000_result_line(uwb_at, "OK"))
    {
      if((uint32_t)(now - role_since) >= role_timeout)
      {
        dw1000_role_failed(now);
      }
      return;
    }
    uwb_collect_at = 0U;
    switch(role_step)
    {
      case ROLE_CACHED: dw1000_role_complete(); break;
      case ROLE_PRECHECK: dw1000_role_step_start(ROLE_SET, now); break;
      case ROLE_SET: dw1000_role_step_start(ROLE_RESET, now); break;
      case ROLE_RESET: role_state = ROLE_REBOOT; role_since = now; break;
      case ROLE_VERIFY:
        if(strcmp(role_target, "TAG") == 0)
        {
          dw1000_role_step_start(ROLE_INTERVAL, now);
        }
        else
        {
          dw1000_role_complete();
        }
        break;
      case ROLE_INTERVAL: dw1000_role_step_start(ROLE_DISPLAY, now); break;
      case ROLE_DISPLAY: dw1000_role_complete(); break;
    }
  }
}

/* Tag T shares the original wire protocol; PC starts with ROLE,TAG and START,TRACK.
 * The TRACK mapping uses A=an2 B=an1 C=an0.
 * Server protocol:
 * ROLE,<seq>,TAG,0\n  or ROLE,<seq>,ANCHOR,<id>\n
 * START,<seq>,ABAC\n / START,<seq>,BC\n / START,<seq>,TRACK\n
 * STOP,<seq>\n ; PING,<seq>\n
 * Reply NODE_A,READY,<seq>,TAG / NODE_A,READY,<seq>,ANCHOR / DIST etc.
 */
void dw1000_execute_command(const char * line)
{
  char copy[CMD_MAX], * p, * seqs, * kind, * id;
  unsigned seq;
  strncpy(copy, line, sizeof(copy) -1U);
  copy[sizeof(copy) -1U] = 0;
  p = strtok(copy, ",");
  seqs = strtok(NULL, ",");
  kind = strtok(NULL, ",");
  id = strtok(NULL, ",");
  if (!p || !seqs) return;
  seq = (unsigned) strtoul(seqs, NULL, 10);
  printf("[PC CMD] %s\r\n", line);
  if (strcmp(p, "ROLE") == 0 && kind && id)
  {
    int n = atoi(id);
    if(!dw1000_role_begin(kind, n, seq, SysTick_GetTick()))
    {
      printf("[ROLE ERROR] step=%s\r\n", uwb_last_failure);
      flow_wifi_send_control("ERROR", seq, "UWB_ROLE_FAILED");
    }
  }
  else if (strcmp(p, "START") == 0 && kind)
  {
    if (strcmp(uwb_role, "TAG") != 0)
    {
      flow_wifi_send_control("ERROR", seq, "NOT_TAG");
      return;
    }
    if (strcmp(kind, "ABAC") && strcmp(kind, "BC") && strcmp(kind, "TRACK"))
    {
      flow_wifi_send_control("ERROR", seq, "BAD_KIND");
      return;
    }
    measuring = 0;
    dw1000_poll_uwb();
    dw1000_ring_clear();
    uwb_line_n = 0;
    uwb_line_overflow = 0U;
    active_seq = seq;
    have_first = 0;
    have_second = 0;
    have_third = 0;
    dw1000_cal_reset(& cal_ab);
    dw1000_cal_reset(& cal_ac);
    dw1000_cal_reset(& cal_bc);
    strncpy(measurement_kind, kind, sizeof(measurement_kind) -1U);
    measurement_kind[sizeof(measurement_kind) -1U] = 0;
    measuring = 1;
    flow_wifi_send_control("STARTED", seq, kind);
  }
  else if (strcmp(p, "STOP") == 0)
  {
    measuring = 0;
    flow_wifi_send_control("STOPPED", seq, "OK");
  }
  else if (strcmp(p, "PING") == 0) flow_wifi_send_control("PONG", seq, "OK");
  else flow_wifi_send_control("ERROR", seq, "BAD_COMMAND");
}


void dw1000_on_disconnect(void)
{
  measuring = 0U;
  dw1000_role_abort();
}

uint8_t dw1000_is_busy(void) { return role_state != ROLE_IDLE; }

void dw1000_init(void)
{
  uwb_w = uwb_r = 0U;
  uwb_drop = 0U;
  measuring = 0U;
  role_state = ROLE_IDLE;
  uwb_line_n = uwb_line_overflow = uwb_at_n = uwb_collect_at = 0U;
  strcpy(uwb_role, "UNKNOWN");
  uwb_anchor_id = -1;
  dw1000_usart_configuration();
  dw1000_tx_cancel(DW1000_USART_PORT, &uwb_tx);
}

void dw1000_process(void)
{
  dw1000_poll_uwb();
  if(!BMC81M001_IsConnected()) return;
  dw1000_role_tick(SysTick_GetTick());
  dw1000_cal_send_pending();
}
