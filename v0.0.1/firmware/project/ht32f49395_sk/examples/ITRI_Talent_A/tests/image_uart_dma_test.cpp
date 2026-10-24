#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ei_main.h"

/* Execute the production start/wait and EI UART functions with simulated DMA.
 * Hardware time advances only when the test or a polling loop requests it.
 */
enum { RESET = 0, TRUE = 1, FALSE = 0, CRM_DMA1_PERIPH_CLOCK = 1,
       FLEX_CHANNEL1 = 1, DMA_FLEXIBLE_UART1_TX = 26,
       DMA_DIR_MEMORY_TO_PERIPHERAL = 1, DMA_PERIPHERAL_DATA_WIDTH_BYTE = 1,
       DMA_MEMORY_DATA_WIDTH_BYTE = 1, DMA_PRIORITY_LOW = 0,
       DMA1_GL1_FLAG = 1, DMA1_FDT1_FLAG = 2, DMA1_DTERR1_FLAG = 8,
       USART_TDC_FLAG = 64, USART_TDBE_FLAG = 128 };
struct dma_init_type {
  uint32_t peripheral_base_addr, memory_base_addr, direction, buffer_size,
           peripheral_inc_enable, memory_inc_enable, peripheral_data_width,
           memory_data_width, loop_mode_enable, priority;
};
struct { uint32_t maddr; } channel;
struct { uint32_t dt; } uart;
#define DMA1 1
#define DMA1_CHANNEL1 (&channel)
#define USART1 (&uart)
#define PRINT_UART USART1
#define EI_BUFFER_SIZE 9216
static uint8_t ei_img_crop_buffer[EI_BUFFER_SIZE];
static uint32_t now, flags, tick_step, dma_started, complete_after;
static bool channel_enabled, request_enabled, stall, drain_stall;
static unsigned configured_bytes, headers, footers, text_bytes;
static bool text_interleaved;
static char wire[40000];
static unsigned wire_size;

static void append(const char *text) {
  size_t length = strlen(text);
  assert(wire_size + length < sizeof(wire));
  memcpy(wire + wire_size, text, length + 1);
  wire_size += length;
}
static void advance_dma() {
  if(channel_enabled && request_enabled && !stall &&
     (uint32_t)(now - dma_started) >= complete_after && !(flags & DMA1_FDT1_FLAG)) {
    assert(configured_bytes == EI_BUFFER_SIZE);
    assert(channel.maddr == (uint32_t)ei_img_crop_buffer);
    assert(wire_size + EI_BUFFER_SIZE < sizeof(wire));
    memcpy(wire + wire_size, ei_img_crop_buffer, EI_BUFFER_SIZE);
    wire_size += EI_BUFFER_SIZE;
    wire[wire_size] = 0;
    flags |= DMA1_FDT1_FLAG;
  }
}
static uint32_t SysTick_GetTick() {
  uint32_t value = now;
  now += tick_step;
  advance_dma();
  return value;
}
static void crm_periph_clock_enable(int, int) {}
static void dma_reset(decltype(DMA1_CHANNEL1)) { channel_enabled = false; }
static void dma_flexible_config(int, int, int) {}
static void dma_default_para_init(dma_init_type *p) { memset(p, 0, sizeof(*p)); }
static void dma_init(decltype(DMA1_CHANNEL1), dma_init_type *p) {
  assert(p->memory_inc_enable && !p->peripheral_inc_enable && !p->loop_mode_enable);
  assert(p->direction == DMA_DIR_MEMORY_TO_PERIPHERAL);
}
static void dma_flag_clear(int) { flags = 0; }
static unsigned dma_flag_get(unsigned flag) { advance_dma(); return flags & flag; }
static void dma_data_number_set(decltype(DMA1_CHANNEL1), unsigned n) { configured_bytes = n; }
static void dma_channel_enable(decltype(DMA1_CHANNEL1), bool enabled) { channel_enabled = enabled; }
static void usart_dma_transmitter_enable(decltype(USART1), bool enabled) {
  request_enabled = enabled;
  if(enabled) dma_started = now;
}
static unsigned usart_flag_get(decltype(USART1), unsigned flag) {
  return flag == USART_TDC_FLAG && drain_stall ? RESET : 1;
}
static void usart_data_transmit(decltype(USART1), uint8_t value) {
  if(request_enabled || headers != footers) text_interleaved = true;
  char text[] = { (char)value, 0 };
  append(text);
  text_bytes++;
}
static int capture_printf(const char *text) {
  if(!strcmp(text, "IMG_S")) { assert(headers == footers); headers++; }
  if(!strcmp(text, "IMG_E\r\n")) {
    assert(!channel_enabled && !request_enabled);
    footers++;
  }
  append(text);
  return (int)strlen(text);
}
#define printf capture_printf
#include "image_uart_under_test.inc"
#undef printf
#include "ei_uart_under_test.inc"

static void reset_case() {
  assert(!image_tx_active);
  now = flags = tick_step = wire_size = headers = footers = text_bytes = 0;
  channel_enabled = request_enabled = stall = drain_stall = text_interleaved = false;
  complete_after = 41;
  wire[0] = 0;
  memset(ei_img_crop_buffer, 'P', sizeof(ei_img_crop_buffer));
  image_uart_dma_init();
}
static void check_packet() {
  assert(!memcmp(wire, "IMG_S", 5));
  assert(!memcmp(wire + 5, ei_img_crop_buffer, EI_BUFFER_SIZE));
  assert(!memcmp(wire + 5 + EI_BUFFER_SIZE, "IMG_E\r\n", 7));
  assert(!text_interleaved);
}
int main() {
  // Start returns while DMA is pending; inference may use the same read-only crop.
  reset_case();
  image_uart_dma_start();
  assert(image_tx_active && request_enabled && channel_enabled && now == 0);
  assert(wire_size == 5 && !(flags & DMA1_FDT1_FLAG));
  now += 180; // Simulated inference runs while DMA transfers the image.
  assert(flow_image_uart_wait());
  assert(now == 180 && footers == 1 && !image_tx_active);
  check_packet();
  assert(flow_image_uart_wait() && footers == 1); // Idempotent completion.

  // Fast inference waits for only the remaining DMA time.
  reset_case(); image_uart_dma_start(); now = 10; tick_step = 1;
  assert(flow_image_uart_wait());
  assert(now >= 41 && now < 50); check_packet();

  // An EI diagnostic must follow the footer, even while inference is running.
  reset_case(); image_uart_dma_start(); tick_step = 1;
  const uint8_t message[] = "diagnostic";
  assert(HT_UART_Transmit(message, sizeof(message) - 1, 5000) == HT_OK);
  assert(text_bytes == sizeof(message) - 1 && footers == 1);
  check_packet();
  assert(!strcmp(wire + 5 + EI_BUFFER_SIZE + 7, "diagnostic"));
  assert(flow_image_uart_wait() && footers == 1);

  // A new start completes the old packet before reloading the DMA channel.
  reset_case(); image_uart_dma_start(); tick_step = 1;
  image_uart_dma_start();
  assert(headers == 2 && footers == 1 && image_tx_active);
  assert(flow_image_uart_wait() && footers == 2);

  // Transfer errors release ownership; the failure survives an early EI drain.
  reset_case(); image_uart_dma_start(); stall = true; flags = DMA1_DTERR1_FLAG;
  assert(!flow_image_uart_wait() && !request_enabled && !channel_enabled);
  assert(!flow_image_uart_wait() && footers == 1);
  stall = false; image_uart_dma_start(); now += 50;
  assert(flow_image_uart_wait()); // Next frame can recover.

  // Timeout is measured from DMA start, including inference time.
  reset_case(); image_uart_dma_start(); stall = true; now = 1001;
  assert(!flow_image_uart_wait() && !image_tx_active && footers == 1);

  // Unsigned time arithmetic works across the millisecond counter rollover.
  reset_case(); now = UINT32_MAX - 20; image_uart_dma_start(); tick_step = 1;
  assert(flow_image_uart_wait()); check_packet();

  // UART drain timeout also releases the DMA channel.
  reset_case(); image_uart_dma_start(); now = 50; drain_stall = true; tick_step = 1;
  assert(!flow_image_uart_wait() && !image_tx_active && !request_enabled);
  puts("PASS: 8 image DMA scenarios (overlap, framing, diagnostics, restart, errors, timeout, wrap, UART drain)");
  return 0;
}
