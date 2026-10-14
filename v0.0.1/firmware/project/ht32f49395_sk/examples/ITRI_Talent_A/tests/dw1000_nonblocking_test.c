/* Runs the real driver with fake UARTs/time on an ARM emulator. */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define _DW1000_H_
#define _BMC81M001_H
#define __HT32F493x5_INT_H
#define TRUE 1
#define FALSE 0
#define SET 1
#define RESET 0
#define USART_TDBE_INT 1
#define USART_RDBF_INT 2
#define USART_TDBE_FLAG 1
#define USART_TDC_FLAG 2
#define USART_RDBF_FLAG 4
#define USART_DATA_8BITS 8
#define USART_STOP_1_BIT 1
#define USART_PARITY_NONE 0
#define GPIO_DRIVE_STRENGTH_STRONGER 1
#define GPIO_OUTPUT_PUSH_PULL 1
#define GPIO_MODE_MUX 1
#define GPIO_PULL_NONE 0
#define GPIO_PULL_UP 1
#define NVIC_PRIORITY_GROUP_4 4
#define DW1000_USART_CLK 1
#define DW1000_USART_TX_GPIO_CLK 1
#define DW1000_USART_RX_GPIO_CLK 1
#define DW1000_USART_TX_GPIO_PORT 1
#define DW1000_USART_RX_GPIO_PORT 1
#define DW1000_USART_TX_PIN 1
#define DW1000_USART_RX_PIN 0
#define DW1000_USART_IRQn 8
#define DW1000_BAUD_RATE 115200U
#define DW1000_RX_BUFFER_SIZE 2048U
#define BMC81M001_RX_BUFFER_SIZE 2048U
#define DW1000_RECONNECT_MS 3000U
#define DW1000_NODE_ID 'A'
#define DW1000_WIFI_SSID "test"
#define DW1000_WIFI_PASSWORD "test"
#define DW1000_SERVER_IP "127.0.0.1"
#define DW1000_SERVER_PORT 5000U
#define __DMB() ((void)0)

typedef struct { int id, tx_enabled, stalled; } usart_type;
typedef struct {
  int gpio_drive_strength, gpio_out_type, gpio_mode, gpio_pins, gpio_pull;
} gpio_init_type;
static usart_type mock_wifi = {0}, mock_uwb = {1};
#define BMC81M001_USART_PORT (&mock_wifi)
#define DW1000_USART_PORT (&mock_uwb)
static uint32_t mock_now;
static char transmitted[2][16384];
static unsigned transmitted_n[2];
static uint8_t mock_rx[4096];
static unsigned mock_rx_w, mock_rx_r;
volatile unsigned test_failure;
volatile unsigned test_number;

static uint32_t SysTick_GetTick(void) { return mock_now; }
static void crm_periph_clock_enable(int clock, int on) { (void)clock; (void)on; }
static void gpio_init(int port, gpio_init_type *config) { (void)port; (void)config; }
static void nvic_priority_group_config(int group) { (void)group; }
static void nvic_irq_enable(int irq, int priority, int sub) { (void)irq; (void)priority; (void)sub; }
static void usart_init(usart_type *p, unsigned baud, int data, int stop) { (void)p; (void)baud; (void)data; (void)stop; }
static void usart_parity_selection_config(usart_type *p, int parity) { (void)p; (void)parity; }
static void usart_transmitter_enable(usart_type *p, int on) { (void)p; (void)on; }
static void usart_receiver_enable(usart_type *p, int on) { (void)p; (void)on; }
static void usart_enable(usart_type *p, int on) { (void)p; (void)on; }
static void usart_interrupt_enable(usart_type *p, int mask, int on)
{
  if(mask == USART_TDBE_INT) p->tx_enabled = on;
}
static int usart_interrupt_flag_get(usart_type *p, int flag)
{
  return flag == USART_TDBE_FLAG && p->tx_enabled && !p->stalled;
}
static int usart_flag_get(usart_type *p, int flag) { (void)flag; return !p->stalled; }
static void usart_data_transmit(usart_type *p, uint16_t data)
{
  if(transmitted_n[p->id] < sizeof(transmitted[0])-1U)
  {
    transmitted[p->id][transmitted_n[p->id]++] = (char)data;
    transmitted[p->id][transmitted_n[p->id]] = 0;
  }
}
static bool BMC81M001_readByte(uint8_t *data)
{
  if(mock_rx_r == mock_rx_w) return false;
  *data = mock_rx[mock_rx_r++ % sizeof(mock_rx)];
  return true;
}
static int test_printf(const char *format, ...) { (void)format; return 0; }
#define printf test_printf
#include "../src/dw1000.c"
#undef printf

#define CHECK(expr) do { if(!(expr)) { test_failure = __LINE__; return 1; } } while(0)

static void pump(void)
{
  unsigned i;
  for(i=0; i<TX_MAX && (mock_wifi.tx_enabled || mock_uwb.tx_enabled); i++)
  {
    dw1000_tx_irq_handler(&mock_wifi);
    dw1000_tx_irq_handler(&mock_uwb);
  }
}
static void service(void) { dw1000_process(); pump(); }
static void tick(uint32_t ms) { mock_now += ms; service(); }
static void wifi_input(const char *text)
{
  while(*text) mock_rx[mock_rx_w++ % sizeof(mock_rx)] = (uint8_t)*text++;
  service();
}
static void uwb_input(const char *text)
{
  while(*text) dw1000_receive_byte((uint8_t)*text++);
  service();
}
static void fixture(void)
{
  mock_now = 0;
  mock_rx_w = mock_rx_r = 0;
  memset(transmitted, 0, sizeof(transmitted));
  memset(transmitted_n, 0, sizeof(transmitted_n));
  mock_wifi.stalled = mock_uwb.stalled = 0;
  dw1000_init();
}
static void connect_wifi(void)
{
  unsigned i;
  for(i=0; i<6; i++)
  {
    service();
    wifi_input("\r\nOK\r\n");
  }
}
static void send_one_event(void)
{
  service();
  wifi_input("\r\n> ");
  wifi_input("\r\nSEND OK\r\n");
}
static void pc_command(const char *text)
{
  char packet[200];
  unsigned i;
  snprintf(packet,sizeof(packet),"+IPD,%u:%s",(unsigned)strlen(text),text);
  /* Deliver every byte separately, exercising parser state across service calls. */
  for(i=0; packet[i]; i++)
  {
    char one[2] = {packet[i],0};
    wifi_input(one);
  }
}
static int test_no_response(void)
{
  unsigned i, length;
  fixture();
  service();
  CHECK(strcmp(transmitted[0],"AT\r\n") == 0);
  CHECK(transmitted_n[1] == 0);
  service();
  for(i=0;i<100;i++) service();
  CHECK(wifi_state == WIFI_REPLY && mock_now == 0);
  tick(2499); CHECK(wifi_state == WIFI_REPLY);
  tick(1); CHECK(wifi_state == WIFI_BACKOFF && !wifi_connected);
  length=transmitted_n[0];
  tick(2999); CHECK(transmitted_n[0] == length);
  tick(1); CHECK(wifi_state == WIFI_TX && transmitted_n[0] == length+4);
  CHECK(transmitted_n[1] == 0);
  return 0;
}
static int test_ap_timeout(void)
{
  unsigned i;
  fixture();
  for(i=0;i<3;i++) { service(); wifi_input("\r\nOK\r\n"); }
  service(); service();
  CHECK(wifi_step == 3 && wifi_state == WIFI_REPLY);
  for(i=0;i<1999;i++) tick(10);
  CHECK(wifi_state == WIFI_REPLY && !wifi_connected);
  tick(10); CHECK(wifi_state == WIFI_BACKOFF);
  CHECK(transmitted_n[1] == 0);
  return 0;
}
static int test_connect_and_ping(void)
{
  fixture(); connect_wifi();
  CHECK(wifi_connected && event_count == 1 && transmitted_n[1] == 0);
  send_one_event();
  CHECK(event_count == 0 && wifi_state == WIFI_ONLINE);
  CHECK(strstr(transmitted[0],"NODE_A,HELLO,0,ONLINE\n") != NULL);
  pc_command("PING,9\n");
  CHECK(event_count == 1);
  CHECK(strstr(events[event_head],"NODE_A,PONG,9,OK\n") != NULL);
  /* CIPSEND may already be queued by the final parser service. */
  if(wifi_state == WIFI_ONLINE) service();
  wifi_input(">"); wifi_input("\r\nSEND OK\r\n");
  CHECK(event_count == 0 && transmitted_n[1] == 0);
  return 0;
}
static int test_role_success(void)
{
  fixture(); connect_wifi(); send_one_event();
  pc_command("ROLE,11,TAG,0\n");
  CHECK(role_state == ROLE_QUIET && transmitted_n[1] == 0);
  tick(219); CHECK(transmitted_n[1] == 0);
  tick(1); CHECK(strcmp(transmitted[1],"AT\r\n") == 0);
  uwb_input("\r\nO"); CHECK(role_state == ROLE_REPLY);
  uwb_input("K"); CHECK(role_state == ROLE_REPLY);
  uwb_input("\r\n"); CHECK(role_step == ROLE_SET);
  tick(220); CHECK(strstr(transmitted[1],"AT+anchor_tag=0\r\n") != NULL);
  uwb_input("\r\nOK\r\n"); tick(220);
  CHECK(strstr(transmitted[1],"AT+RST\r\n") != NULL);
  uwb_input("\r\nOK\r\n"); CHECK(role_state == ROLE_REBOOT);
  tick(1699); CHECK(role_state == ROLE_REBOOT);
  tick(1); CHECK(role_step == ROLE_VERIFY);
  tick(220); uwb_input("\r\nOK\r\n");
  CHECK(role_step == ROLE_INTERVAL);
  tick(220); uwb_input("\r\nOK\r\n");
  CHECK(role_step == ROLE_DISPLAY);
  tick(220); uwb_input("\r\nOK\r\n");
  CHECK(role_state == ROLE_IDLE && strcmp(uwb_role,"TAG") == 0);
  CHECK(strstr(events[event_head],"NODE_A,READY,11,TAG\n") != NULL);
  return 0;
}
static int test_role_timeout(void)
{
  unsigned i;
  fixture(); connect_wifi(); send_one_event(); pc_command("ROLE,12,TAG,0\n");
  tick(220);
  for(i=0;i<3;i++)
  {
    service(); CHECK(role_state == ROLE_REPLY);
    tick(1500);
    if(i<2) { CHECK(role_state == ROLE_RETRY); tick(450); tick(220); }
  }
  CHECK(role_state == ROLE_IDLE && strcmp(uwb_role,"UNKNOWN") == 0);
  CHECK(strstr(events[event_head],"NODE_A,ERROR,12,UWB_ROLE_FAILED\n") != NULL);
  CHECK(strcmp(transmitted[1],"AT\r\nAT\r\nAT\r\n") == 0);
  return 0;
}
static int test_disconnect(void)
{
  unsigned length;
  fixture(); connect_wifi(); send_one_event(); pc_command("ROLE,13,TAG,0\n");
  wifi_input("\r\nCLOSED\r\n");
  CHECK(!wifi_connected && !measuring && role_state == ROLE_IDLE && event_count == 0);
  length=transmitted_n[0];
  tick(2999); CHECK(transmitted_n[0] == length && transmitted_n[1] == 0);
  tick(1); CHECK(wifi_state == WIFI_TX);
  return 0;
}
static int test_queue_and_median(void)
{
  unsigned i;
  const float values[] = {90,91,5,1,4,2,3};
  fixture(); wifi_connected=1; wifi_state=WIFI_ONLINE; measuring=1;
  dw1000_cal_reset(&cal_ab);
  for(i=0;i<7;i++) dw1000_cal_add(&cal_ab,values[i]);
  CHECK(cal_ab.pending && cal_ab.median == 3.0f);
  for(i=0;i<EVENT_COUNT;i++) CHECK(dw1000_send_event("PONG",i,"OK"));
  CHECK(!dw1000_send_event("PONG",99,"FULL"));
  dw1000_cal_emit(&cal_ab,"AB"); CHECK(cal_ab.pending && !cal_ab.sent);
  event_count--;
  dw1000_cal_emit(&cal_ab,"AB"); CHECK(!cal_ab.pending && cal_ab.sent);
  CHECK(event_count == EVENT_COUNT);
  return 0;
}
static int test_stalled_tx_and_wrap(void)
{
  fixture(); mock_wifi.stalled=1; service();
  CHECK(wifi_state == WIFI_TX);
  tick(1000); CHECK(wifi_state == WIFI_BACKOFF && !mock_wifi.tx_enabled);
  fixture(); mock_now=UINT32_MAX-100U; service(); service();
  tick(2500); CHECK(wifi_state == WIFI_BACKOFF);
  tick(3000); CHECK(wifi_state == WIFI_TX);
  return 0;
}
static int test_tcp_timeout_and_payload(void)
{
  unsigned i;
  fixture();
  for(i=0;i<5;i++) { service(); wifi_input("\r\nOK\r\n"); }
  service(); service(); tick(9999); CHECK(wifi_state == WIFI_REPLY);
  tick(1); CHECK(wifi_state == WIFI_BACKOFF);
  fixture(); connect_wifi(); service(); service();
  CHECK(wifi_operation == WIFI_SEND_PROMPT && wifi_state == WIFI_REPLY);
  /* ERROR inside an IPD payload must not fail the AT transaction. */
  wifi_input("+IPD,5:ERROR");
  CHECK(wifi_connected && wifi_state == WIFI_REPLY);
  tick(4000); CHECK(!wifi_connected && wifi_state == WIFI_BACKOFF);
  return 0;
}

static int test_ap_rejoin_notifications(void)
{
  unsigned split, i;
  for(split=0;split<2;split++)
  {
    fixture();
    for(i=0;i<3;i++) { service(); wifi_input("\r\nOK\r\n"); }
    service();
    CHECK(wifi_step == 3 && wifi_state == WIFI_TX);
    if(split)
    {
      wifi_input("\r\nWIFI DISCONNECT\r\n");
      CHECK(wifi_state == WIFI_REPLY && wifi_step == 3);
      wifi_input("WIFI CONNECTED\r\n");
      wifi_input("WIFI GOT IP\r\n");
      CHECK(wifi_state == WIFI_REPLY);
      wifi_input("\r\nOK\r\n");
    }
    else
    {
      wifi_input("\r\nWIFI DISCONNECT\r\nWIFI CONNECTED\r\nWIFI GOT IP\r\n\r\nOK\r\n");
    }
    CHECK(wifi_state == WIFI_START && wifi_step == 4);
    service(); CHECK(strstr(transmitted[0],"AT+CIPMUX=0\r\n") != NULL);
    wifi_input("\r\nOK\r\n");
    service(); CHECK(strstr(transmitted[0],"AT+CIPSTART=\"TCP\",\"127.0.0.1\",5000\r\n") != NULL);
    wifi_input("\r\nCONNECT\r\n\r\nOK\r\n");
    CHECK(wifi_connected && event_count == 1 && transmitted_n[1] == 0);
  }
  return 0;
}

static int test_setup_disconnect_and_failure(void)
{
  unsigned i;
  fixture();
  for(i=0;i<2;i++) { service(); wifi_input("\r\nOK\r\n"); }
  service(); wifi_input("\r\nWIFI DISCONNECT\r\n\r\nOK\r\n");
  CHECK(wifi_state == WIFI_START && wifi_step == 3);
  service(); wifi_input("\r\nWIFI DISCONNECT\r\n");
  CHECK(wifi_state == WIFI_REPLY);
  wifi_input("\r\nFAIL\r\n");
  CHECK(wifi_state == WIFI_BACKOFF && !wifi_connected);
  return 0;
}

static int test_link_loss_after_ap_join(void)
{
  unsigned i;
  fixture();
  for(i=0;i<5;i++) { service(); wifi_input("\r\nOK\r\n"); }
  service(); CHECK(wifi_step == 5 && wifi_state == WIFI_TX);
  wifi_input("\r\nWIFI DISCONNECT\r\n");
  CHECK(wifi_state == WIFI_BACKOFF && !wifi_connected);
  fixture(); connect_wifi(); send_one_event();
  wifi_input("\r\nWIFI DISCONNECT\r\n");
  CHECK(wifi_state == WIFI_BACKOFF && !wifi_connected);
  return 0;
}

int main(void)
{
  int (*tests[])(void) = {test_no_response,test_ap_timeout,test_connect_and_ping,
    test_role_success,test_role_timeout,test_disconnect,test_queue_and_median,
    test_stalled_tx_and_wrap,test_tcp_timeout_and_payload,test_ap_rejoin_notifications,
    test_setup_disconnect_and_failure,test_link_loss_after_ap_join};
  for(test_number=0;test_number<sizeof(tests)/sizeof(tests[0]);test_number++)
  {
    if(tests[test_number]()) return 1;
  }
  return 0;
}
