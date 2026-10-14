#include "dw1000.h"
#include "bmc81m001.h"
#include "ht32f493x5_int.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UART_MASK (DW1000_RX_BUFFER_SIZE - 1U)
#define CMD_MAX 120U
#define UWB_LINE_MAX 160U
#define TX_MAX 256U
#define EVENT_MAX 200U
#define EVENT_COUNT 8U
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

static dw1000_tx_t wifi_tx, uwb_tx;

/* Main context owns the buffer until it is published to the TX interrupt. */
static uint8_t dw1000_tx_start(usart_type *port, dw1000_tx_t *tx, const char *text)
{
  size_t length = strlen(text);
  if(length == 0U || length >= TX_MAX || tx->position < tx->length)
  {
    return 0U;
  }
  usart_interrupt_enable(port, USART_TDBE_INT, FALSE);
  memcpy(tx->data, text, length);
  tx->position = 0U;
  tx->length = (uint16_t)length;
  __DMB();
  usart_interrupt_enable(port, USART_TDBE_INT, TRUE);
  return 1U;
}

void dw1000_tx_irq_handler(usart_type *port)
{
  dw1000_tx_t *tx = (port == DW1000_USART_PORT) ? &uwb_tx : &wifi_tx;
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

static int dw1000_ring_get(uint8_t wifi)
{
  uint8_t data;
  if(wifi)
  {
    return BMC81M001_readByte(&data) ? (int)data : -1;
  }
  if(uwb_r == uwb_w)
  {
    return -1;
  }
  data = uwb_ring[uwb_r];
  uwb_r = (uint16_t)((uwb_r + 1U) & UART_MASK);
  return (int)data;
}

static void dw1000_ring_clear(uint8_t wifi)
{
  if(!wifi)
  {
    uwb_r = uwb_w;
  }
}

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

/* Wi-Fi parser: ESP8266 +IPD,<length>:<data>. Handles fragmentation.
 * Only AT command replies are recorded outside +IPD payload.
 */
static char wifi_at[512];
static uint16_t at_n = 0;
static char ipd_head[30];
static uint8_t ipd_hn = 0, ipd_mode = 0;
/* 0 normal, 1 header, 2 payload */
static uint16_t ipd_remaining = 0;
static uint8_t ipd_match;
static char command[CMD_MAX];
static uint16_t command_n = 0;
static uint8_t command_overflow;
static char pending_command[CMD_MAX];
static uint8_t command_ready = 0, wifi_connected = 0;
static uint8_t measuring = 0;
static unsigned active_seq = 0;
static uint8_t have_first = 0, have_second = 0;
static char measurement_kind[8];
static uint8_t have_third = 0;
static void dw1000_command_byte(uint8_t b)
{
  if (b == '\r') return;
  if (b == '\n')
  {
    if (command_n && !command_ready && !command_overflow)
    {
      command[command_n] = 0;
      strcpy(pending_command, command);
      command_ready = 1;
    }
    command_n = 0;
    command_overflow = 0;
    return;
  }
  if (command_overflow) return;
  if (command_n < CMD_MAX -1U) command[command_n++] = (char) b;
  else
  {
    command_n = 0;
    command_overflow = 1U;
  }
}

static void dw1000_wifi_at_byte(char b)
{
  if (at_n >= sizeof(wifi_at) -1U)
  {
    memmove(wifi_at, wifi_at + 128U, sizeof(wifi_at) -129U);
    at_n = (uint16_t) (sizeof(wifi_at) -129U);
  }
  wifi_at[at_n++] = b;
  wifi_at[at_n] = '\0';
}

static void dw1000_wifi_byte(uint8_t b)
{
  const char * prefix = "+IPD,";
  if (ipd_mode == 2)
  {
    dw1000_command_byte(b);
    if (--ipd_remaining == 0U) ipd_mode = 0;
    return;
  }
  if (ipd_mode == 1)
  {
    if (b == ':')
    {
      unsigned long n;
      ipd_head[ipd_hn] = 0;
      /* CIPMUX=0: +IPD,len; optional +IPD,0,len accepted. */
      {
        char * p = strrchr(ipd_head, ',');
        n = strtoul(p ? p + 1 : ipd_head, NULL, 10);
      }
      ipd_remaining = (uint16_t) ((n > 4096U) ? 0U : n);
      ipd_mode = (ipd_remaining > 0U) ? 2U : 0U;
      ipd_hn = 0;
      return;
    }
    if (ipd_hn < sizeof(ipd_head) -1U && ((b >= '0' && b <= '9') || b == ','))
    {
      ipd_head[ipd_hn++] = (char) b;
      return;
    }
    /* Malformed header: discard and recover. */
    ipd_mode = 0;
    ipd_hn = 0;
  }
  /* Scan normal text for +IPD, while retaining AT responses. */
  {
    if (b == (uint8_t) prefix[ipd_match])
    {
      ipd_match++;
      if (ipd_match == 5U)
      {
        ipd_mode = 1;
        ipd_hn = 0;
        ipd_match = 0;
        return;
      }
    }
    else ipd_match = (b == '+') ? 1U : 0U;
  }
  dw1000_wifi_at_byte((char) b);
}

static void dw1000_poll_wifi(void)
{
  int b;
  uint16_t budget = BMC81M001_RX_BUFFER_SIZE;
  while(budget-- != 0U && (b = dw1000_ring_get(1)) >= 0)
  {
    dw1000_wifi_byte((uint8_t)b);
  }
}

static void dw1000_reset_at_capture(void)
{
  at_n = 0;
  wifi_at[0] = 0;
}

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


typedef enum
{
  WIFI_START, WIFI_TX, WIFI_REPLY, WIFI_ONLINE, WIFI_BACKOFF
} dw1000_wifi_state_t;

typedef enum
{
  WIFI_CONNECT, WIFI_SEND_PROMPT, WIFI_SEND_ACK
} dw1000_wifi_operation_t;

static dw1000_wifi_state_t wifi_state;
static dw1000_wifi_operation_t wifi_operation;
static uint8_t wifi_step;
static uint32_t wifi_since, wifi_timeout;
static char events[EVENT_COUNT][EVENT_MAX];
static uint8_t event_head, event_tail, event_count;
static void dw1000_role_abort(void);

static uint8_t dw1000_send_event(const char *kind, unsigned seq, const char *value)
{
  int length;
  if(!wifi_connected || event_count >= EVENT_COUNT)
  {
    return 0U;
  }
  length = snprintf(events[event_tail], EVENT_MAX, "NODE_%c,%s,%u,%s\n",
                    DW1000_NODE_ID, kind, seq, value);
  if(length < 0 || length >= (int)EVENT_MAX)
  {
    return 0U;
  }
  event_tail = (uint8_t)((event_tail + 1U) % EVENT_COUNT);
  event_count++;
  return 1U;
}

static void dw1000_wifi_disconnect(uint32_t now)
{
  dw1000_tx_cancel(BMC81M001_USART_PORT, &wifi_tx);
  wifi_connected = 0U;
  measuring = 0U;
  command_ready = 0U;
  command_n = 0U;
  command_overflow = 0U;
  ipd_mode = ipd_hn = ipd_match = 0U;
  ipd_remaining = 0U;
  event_head = event_tail = event_count = 0U;
  wifi_state = WIFI_BACKOFF;
  wifi_since = now;
  dw1000_reset_at_capture();
  dw1000_role_abort();
}

/* Stage names deliberately omit AP credentials. */
static const char *dw1000_wifi_stage(void)
{
  static const char *const stages[] = {"AT", "ATE0", "CWMODE", "CWJAP", "CIPMUX", "CIPSTART"};
  if(wifi_operation == WIFI_SEND_PROMPT)
  {
    return "CIPSEND prompt";
  }
  if(wifi_operation == WIFI_SEND_ACK)
  {
    return "CIPSEND acknowledgement";
  }
  return wifi_step < 6U ? stages[wifi_step] : "TCP";
}

static void dw1000_wifi_fail(uint32_t now, const char *reason)
{
  printf("[WIFI] %s: %s; retry in %lu ms\r\n", dw1000_wifi_stage(), reason,
         (unsigned long)DW1000_RECONNECT_MS);
  dw1000_wifi_disconnect(now);
}

static uint8_t dw1000_wifi_begin(const char *text, dw1000_wifi_operation_t operation,
                                uint32_t timeout, uint32_t now)
{
  dw1000_reset_at_capture();
  if(!dw1000_tx_start(BMC81M001_USART_PORT, &wifi_tx, text))
  {
    dw1000_wifi_fail(now, "TX unavailable");
    return 0U;
  }
  wifi_operation = operation;
  wifi_timeout = timeout;
  wifi_since = now;
  wifi_state = WIFI_TX;
  return 1U;
}

static void dw1000_wifi_connect_step(uint32_t now)
{
  char cmd[TX_MAX];
  uint32_t timeout = 3000U;
  int length = 0;
  switch(wifi_step)
  {
    case 0: length = snprintf(cmd, sizeof(cmd), "AT\r\n"); timeout = 2500U; break;
    case 1: length = snprintf(cmd, sizeof(cmd), "ATE0\r\n"); timeout = 2500U; break;
    case 2: length = snprintf(cmd, sizeof(cmd), "AT+CWMODE=1\r\n"); break;
    case 3:
      length = snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n",
                        DW1000_WIFI_SSID, DW1000_WIFI_PASSWORD);
      timeout = 20000U;
      break;
    case 4: length = snprintf(cmd, sizeof(cmd), "AT+CIPMUX=0\r\n"); break;
    case 5:
      length = snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u\r\n",
                        DW1000_SERVER_IP, (unsigned)DW1000_SERVER_PORT);
      timeout = 10000U;
      break;
    default:
      dw1000_wifi_disconnect(now);
      return;
  }
  if(length <= 0 || length >= (int)sizeof(cmd))
  {
    dw1000_wifi_disconnect(now);
    return;
  }
  wifi_operation = WIFI_CONNECT;
  if(wifi_step == 5U)
  {
    printf("[WIFI] CIPSTART TCP %s:%u\r\n", DW1000_SERVER_IP, (unsigned)DW1000_SERVER_PORT);
  }
  else
  {
    printf("[WIFI] %s\r\n", dw1000_wifi_stage());
  }
  dw1000_wifi_begin(cmd, WIFI_CONNECT, timeout, now);
}

static void dw1000_wifi_tick(uint32_t now)
{
  char cmd[40];
  uint8_t success;
  if(wifi_state == WIFI_BACKOFF)
  {
    if((uint32_t)(now - wifi_since) < DW1000_RECONNECT_MS)
    {
      return;
    }
    wifi_step = 0U;
    wifi_state = WIFI_START;
  }
  if(wifi_state == WIFI_START)
  {
    dw1000_wifi_connect_step(now);
    return;
  }
  if(wifi_state == WIFI_TX)
  {
    if(dw1000_tx_done(BMC81M001_USART_PORT, &wifi_tx))
    {
      wifi_state = WIFI_REPLY;
      wifi_since = now;
    }
    else
    {
      if((uint32_t)(now - wifi_since) >= TX_TIMEOUT_MS)
      {
        dw1000_wifi_fail(now, "UART TX timeout");
      }
      return;
    }
  }
  if(wifi_state == WIFI_REPLY)
  {
    success = 0U;
    /* ALREADY CONNECT may be followed by ERROR on some ESP-AT versions. */
    if(wifi_operation == WIFI_CONNECT && wifi_step == 5U &&
       dw1000_result_line(wifi_at, "ALREADY CONNECT"))
    {
      success = 1U;
    }
    if(!success && (dw1000_result_line(wifi_at, "ERROR") ||
                   dw1000_result_line(wifi_at, "FAIL") ||
                   dw1000_result_line(wifi_at, "SEND FAIL") || strstr(wifi_at, "busy p")))
    {
      const char *reason = strstr(wifi_at, "busy p") ? "module busy" :
                           dw1000_result_line(wifi_at, "ERROR") ? "ERROR reply" : "FAIL reply";
      dw1000_wifi_fail(now, reason);
      return;
    }
    if(wifi_operation == WIFI_CONNECT)
    {
      success = success || dw1000_result_line(wifi_at, "OK");
    }
    else if(wifi_operation == WIFI_SEND_PROMPT)
    {
      success = strstr(wifi_at, ">") != NULL;
    }
    else
    {
      success = dw1000_result_line(wifi_at, "SEND OK");
    }
    if(!success)
    {
      if((uint32_t)(now - wifi_since) >= wifi_timeout)
      {
        dw1000_wifi_fail(now, "reply timeout");
      }
      return;
    }
    if(wifi_operation == WIFI_CONNECT)
    {
      wifi_step++;
      if(wifi_step == 6U)
      {
        wifi_connected = 1U;
        printf("[WIFI] TCP connected %s:%u\r\n", DW1000_SERVER_IP, (unsigned)DW1000_SERVER_PORT);
        wifi_state = WIFI_ONLINE;
        dw1000_send_event("HELLO", 0U, "ONLINE");
      }
      else
      {
        wifi_state = WIFI_START;
      }
      dw1000_reset_at_capture();
    }
    else if(wifi_operation == WIFI_SEND_PROMPT)
    {
      dw1000_wifi_begin(events[event_head], WIFI_SEND_ACK, 5000U, now);
    }
    else
    {
      event_head = (uint8_t)((event_head + 1U) % EVENT_COUNT);
      event_count--;
      wifi_state = WIFI_ONLINE;
      dw1000_reset_at_capture();
    }
    return;
  }
  if(wifi_state == WIFI_ONLINE && event_count != 0U)
  {
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u\r\n", (unsigned)strlen(events[event_head]));
    dw1000_wifi_begin(cmd, WIFI_SEND_PROMPT, 4000U, now);
  }
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
  if(dw1000_send_event("DIST", active_seq, value))
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
          if(dw1000_send_event("DIST", active_seq, value))
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
  while(budget-- != 0U && (b = dw1000_ring_get(0)) >= 0)
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
  dw1000_send_event("READY", role_seq, role_target);
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
  dw1000_send_event("ERROR", role_seq, "UWB_ROLE_FAILED");
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
    dw1000_ring_clear(0U);
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
static void dw1000_execute_command(const char * line)
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
      dw1000_send_event("ERROR", seq, "UWB_ROLE_FAILED");
    }
  }
  else if (strcmp(p, "START") == 0 && kind)
  {
    if (strcmp(uwb_role, "TAG") != 0)
    {
      dw1000_send_event("ERROR", seq, "NOT_TAG");
      return;
    }
    if (strcmp(kind, "ABAC") && strcmp(kind, "BC") && strcmp(kind, "TRACK"))
    {
      dw1000_send_event("ERROR", seq, "BAD_KIND");
      return;
    }
    measuring = 0;
    dw1000_poll_uwb();
    dw1000_ring_clear(0);
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
    dw1000_send_event("STARTED", seq, kind);
  }
  else if (strcmp(p, "STOP") == 0)
  {
    measuring = 0;
    dw1000_send_event("STOPPED", seq, "OK");
  }
  else if (strcmp(p, "PING") == 0) dw1000_send_event("PONG", seq, "OK");
  else dw1000_send_event("ERROR", seq, "BAD_COMMAND");
}


void dw1000_init(void)
{
  uwb_w = uwb_r = 0U;
  uwb_drop = 0U;
  wifi_connected = 0U;
  measuring = 0U;
  wifi_step = 0U;
  wifi_state = WIFI_START;
  role_state = ROLE_IDLE;
  event_head = event_tail = event_count = 0U;
  command_ready = 0U;
  command_n = 0U;
  command_overflow = 0U;
  ipd_mode = ipd_hn = ipd_match = 0U;
  ipd_remaining = 0U;
  uwb_line_n = uwb_line_overflow = uwb_at_n = uwb_collect_at = 0U;
  strcpy(uwb_role, "UNKNOWN");
  uwb_anchor_id = -1;
  dw1000_reset_at_capture();
  dw1000_usart_configuration();
  dw1000_tx_cancel(DW1000_USART_PORT, &uwb_tx);
  dw1000_tx_cancel(BMC81M001_USART_PORT, &wifi_tx);
}

void dw1000_process(void)
{
  uint32_t now = SysTick_GetTick();
  dw1000_poll_wifi();
  /* CWMODE/CWJAP can disconnect an old AP before CONNECTED/GOT IP/OK.
     During AP setup, let the command's final reply or timeout decide success.
     After CWJAP succeeds, link loss must still cancel TCP/measurement work. */
  if((wifi_connected || (wifi_step >= 4U && wifi_state != WIFI_BACKOFF)) &&
     (dw1000_result_line(wifi_at, "CLOSED") ||
      dw1000_result_line(wifi_at, "WIFI DISCONNECT")))
  {
    dw1000_wifi_fail(now, "connection lost");
  }
  dw1000_wifi_tick(now);
  if(!wifi_connected)
  {
    dw1000_poll_uwb();
    return;
  }
  /* Reserve reply space; the PC sends the next command after its response. */
  if(command_ready && role_state == ROLE_IDLE && event_count <= EVENT_COUNT - 4U)
  {
    char local[CMD_MAX];
    strcpy(local, pending_command);
    command_ready = 0U;
    dw1000_execute_command(local);
  }
  dw1000_poll_uwb();
  dw1000_role_tick(now);
  dw1000_cal_send_pending();
}
