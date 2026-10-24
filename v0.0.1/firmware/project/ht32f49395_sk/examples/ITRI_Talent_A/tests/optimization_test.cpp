#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
struct { uint32_t LOAD, VAL; } systick;
struct { uint32_t ICSR; } scb;
#define SysTick (&systick)
#define SCB (&scb)
#define SCB_ICSR_PENDSTSET_Msk (1UL << 26)
static uint32_t primask, uwTick, tick_epoch;
static uint32_t __get_PRIMASK() { return primask; }
static void __disable_irq() { primask = 1; }
static void __set_PRIMASK(uint32_t p) { primask = p; }
#include "time_under_test.inc"
static char output[256];
static unsigned output_length;
static int HT_UART_Transmit(const uint8_t *p, uint16_t size, uint32_t) {
  assert(size < sizeof(output));
  memcpy(output,p,size); output[size]=0; output_length=size;
  return 0;
}
#include "format_under_test.inc"
int main() {
  systick.LOAD=239999; systick.VAL=119999; uwTick=100;
  assert(SysTick_GetTimeUs()==100500 && primask==0);
  // Prior 32-bit ms*1000 overflow at ~71.6 minutes.
  uwTick=4294968; assert(SysTick_GetTimeUs()==4294968500ULL);
  // A tick pending before its ISR has run must be reflected in the snapshot.
  uwTick=99; scb.ICSR=SCB_ICSR_PENDSTSET_Msk;
  assert(SysTick_GetTimeUs()==100500);
  systick.VAL=0; assert(SysTick_GetTimeUs()==100000);
  uwTick=UINT32_MAX; systick.VAL=systick.LOAD;
  assert(SysTick_GetTimeUs()==(1ULL<<32)*1000ULL);
  scb.ICSR=0; tick_epoch=1; uwTick=0; primask=1;
  assert(SysTick_GetTimeUs()==(1ULL<<32)*1000ULL && primask==1);
  // Formatting is bounded for finite values, NaN/Inf and oversized diagnostics.
  ei_printf_float(0.125f); assert(!strcmp(output,"0.12500"));
  ei_printf_float(-1.0f); assert(!strcmp(output,"-1.00000"));
  ei_printf_float(INFINITY); assert(strstr(output,"inf"));
  ei_printf_float(-INFINITY); assert(!strcmp(output,"-inf"));
  ei_printf_float(NAN); assert(strstr(output,"nan"));
  ei_printf_float(1e38f); assert(output_length<32);
  char long_message[600]; memset(long_message,'x',599); long_message[599]=0;
  ei_printf("%s",long_message); assert(output_length==255);
  puts("PASS: microsecond timing, pending tick, 32/64-bit rollover and bounded float/text formatting");
}
