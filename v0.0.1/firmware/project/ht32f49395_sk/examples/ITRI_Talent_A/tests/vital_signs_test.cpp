/* Real UART parser, reference-state logic, telemetry and fall buzzer flow.
   The runner removes only hardware initialization and SDK-only declarations. */
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "flow_metrics.h"
flow_metrics_t g_flow_metrics;
static uint32_t fake_now;
static uint32_t SysTick_GetTick(void) { return fake_now; }
#include "mr60bha2_under_test.inc"

#define EI_BUFFER_SIZE 9216U
static uint8_t ei_img_crop_buffer[EI_BUFFER_SIZE];
static unsigned fall_queue_calls;
static bool reject_fall_queue;
static struct { uint8_t level, valid, first_pixel; float heart, breath; } queued_falls[8];
static uint8_t flow_wifi_queue_fall_event(uint8_t level, uint8_t valid,
                                      float heart, float breath, const uint8_t *image, uint32_t size)
{
    if(reject_fall_queue || fall_queue_calls >= 8 || size != EI_BUFFER_SIZE) return 0;
    queued_falls[fall_queue_calls].level = level;
    queued_falls[fall_queue_calls].valid = valid;
    queued_falls[fall_queue_calls].heart = heart;
    queued_falls[fall_queue_calls].breath = breath;
    queued_falls[fall_queue_calls].first_pixel = image[0];
    ++fall_queue_calls;
    return 1;
}

static unsigned reports;
static bool buzzer;
static char telemetry[256];
static size_t telemetry_used;
static int test_printf(const char *fmt, ...) { if(strncmp(fmt, "REPORT:", 7) == 0) ++reports; return 0; }
static void ei_printf(const char *fmt, ...)
{
    va_list args; va_start(args, fmt);
    telemetry_used += vsnprintf(telemetry + telemetry_used,
        sizeof(telemetry) - telemetry_used, fmt, args);
    va_end(args);
}
static void ei_printf_float(float value) { ei_printf("%.2f", (double)value); }
#define printf test_printf
#define GPIOB 0
#define GPIO_PINS_1 1
#define EI_CLASSIFIER_OBJECT_DETECTION 1
static void gpio_bits_reset(int, int) { buzzer = false; }
static void gpio_bits_set(int, int) { buzzer = true; }
typedef struct { const char *label; float value; } box_t;
typedef struct { uint32_t bounding_boxes_count; box_t bounding_boxes[2]; } ei_impulse_result_t;
#include "vital_signs_flow_under_test.inc"
#undef printf

volatile unsigned test_number, test_failure;
#define CHECK(c) do { if(!(c)) { test_failure = __LINE__; return 1; } } while(0)
static void reset(void) { fake_now = 0; mr60bha2_init(); reports = 0; fall_queue_calls = 0; }
static void u32(uint8_t *p, uint32_t n)
{
    for(unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(n >> (8U * i));
}
static void f32(uint8_t *p, float f) { memcpy(p, &f, 4); }
static void emit(uint16_t type, const uint8_t *data, unsigned len, bool corrupt = false)
{
    uint8_t h[8] = {1, 0, 0, (uint8_t)(len >> 8), (uint8_t)len,
                   (uint8_t)(type >> 8), (uint8_t)type, 0};
    uint8_t c = 0;
    for(unsigned i = 0; i < 7; ++i) c ^= h[i];
    h[7] = (uint8_t)~c;
    for(unsigned i = 0; i < 8; ++i) mr60bha2_receive_byte(h[i]);
    c = 0;
    for(unsigned i = 0; i < len; ++i) { c ^= data[i]; mr60bha2_receive_byte(data[i]); }
    mr60bha2_receive_byte((uint8_t)~c ^ (corrupt ? 1U : 0U));
}
static void presence(uint8_t value)
{
    // Board capture: presence is emitted as two payload bytes.
    uint8_t payload[2] = {value, 0};
    emit(MR60BHA2_TYPE_HUMAN_DETECTION, payload, sizeof(payload));
}
static void distance(float cm, uint32_t flag = 1)
{
    uint8_t p[8]; u32(p, flag); f32(p+4, cm); emit(MR60BHA2_TYPE_DISTANCE, p, 8);
}
static void targets(uint32_t count = 1, int32_t doppler = 0, uint32_t cluster = 7)
{
    uint8_t p[52] = {}; u32(p, count);
    for(unsigned i = 0; i < count && i < 3; ++i)
    {
        f32(p + 4 + 16*i, 0.0f); f32(p + 8 + 16*i, 1.0f);
        u32(p + 12 + 16*i, (uint32_t)doppler); u32(p + 16 + 16*i, cluster+i);
    }
    emit(MR60BHA2_TYPE_TARGET_INFO, p, 4 + 16 * count);
}
static void rates(float breath = 20, float heart = 80)
{
    uint8_t p[4]; f32(p, breath); emit(MR60BHA2_TYPE_BREATH_RATE, p, 4);
    f32(p, heart); emit(MR60BHA2_TYPE_HEART_RATE, p, 4);
}
static void sample(uint32_t now, float breath = 20, float heart = 80)
{
    fake_now = now; presence(1); distance(100); targets(); rates(breath, heart);
    mr60bha2_process();
}
static MR60BHA2_Vitals_t view(void)
{
    MR60BHA2_Vitals_t result; mr60bha2_get_vitals(&result); return result;
}
static void ready(void)
{
    reset();
    for(unsigned i = 0; i <= 3; ++i) sample(i * 1000U);
}
static const char *wire(void)
{
    telemetry_used = 0; telemetry[0] = 0; flow_send_vital_signs(); return telemetry;
}
int main(void)
{
    reset();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    presence(0); rates(); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_ABSENT);
    CHECK(mr60bha2_data.heart_rate == 0 && heart_window.count == 0);
    CHECK(strstr(wire(), "heart=N/A breath=N/A status=absent"));
    ++test_number;

    reset();
    for(unsigned i = 0; i < 3; ++i) { sample(i*1000U); CHECK(view().status == MR60BHA2_VITAL_INVALID); }
    sample(3000);
    CHECK(view().status == MR60BHA2_VITAL_REFERENCE);
    CHECK(view().heart_rate == 80 && view().breath_rate == 20);
    CHECK(strstr(wire(), "heart=80.00 breath=20.00 status=reference"));
    // Previously abnormal values remain reference-only and cannot start a buzzer.
    for(unsigned i = 4; i <= 20; ++i) sample(i*1000U, 5, 200);
    flow_buzzer_tick(fake_now);
    CHECK(view().status == MR60BHA2_VITAL_REFERENCE && !buzzer && reports == 0);
    ++test_number;

    ready(); fake_now = 3100; targets(2); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_MULTIPLE && heart_window.count == 0);
    CHECK(strstr(wire(), "heart=N/A breath=N/A status=multiple"));
    // Multiple -> single in the same processing batch still discards old samples.
    targets(); rates(); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    sample(4100); sample(5100); CHECK(view().status == MR60BHA2_VITAL_INVALID);
    sample(6100); CHECK(view().status == MR60BHA2_VITAL_REFERENCE);
    ++test_number;

    ready(); fake_now = 3100; targets(1, -1); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID && heart_window.count == 0);
    targets(1, 0); rates(); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    ++test_number;

    ready(); fake_now = 3100; distance(150); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_REFERENCE);
    distance(150.01f); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    distance(100, 0); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    distance(NAN); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    ++test_number;

    // Fresh rates cannot hide stale target information.
    ready();
    for(unsigned i = 4; i <= 6; ++i)
    {
        fake_now = i*1000U; presence(1); distance(100); rates(); mr60bha2_process();
    }
    CHECK(view().status == MR60BHA2_VITAL_INVALID && heart_window.count == 0);
    ready();
    for(unsigned i = 4; i <= 6; ++i)
    {
        fake_now = i*1000U; presence(1); distance(100); targets();
        uint8_t p[4]; f32(p, 20); emit(MR60BHA2_TYPE_BREATH_RATE, p, 4);
        mr60bha2_process();
    }
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    ready(); fake_now = 6000;
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    ++test_number;

    ready(); fake_now = 3100; rates(0, 0); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    rates(NAN, INFINITY); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    rates(-1, -1); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    CHECK(strstr(wire(), "heart=N/A breath=N/A status=invalid"));
    flow_buzzer_tick(fake_now); CHECK(!buzzer && reports == 0);
    ++test_number;

    ready(); fake_now = 3100; presence(0); presence(1); rates(); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID);
    ready(); fake_now = 3100; targets(1, 0, 99); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID && heart_window.count == 0);
    ++test_number;

    // Bounds/endianness validation; raw point clouds are not people counts.
    ready(); fake_now = 3100;
    uint8_t invalid[4] = {0, 0, 0, 1}; // Incorrect big endian count = 16777216.
    emit(MR60BHA2_TYPE_TARGET_INFO, invalid, 4); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID && !context.targets_received);
    u32(invalid, 2); emit(MR60BHA2_TYPE_TARGET_INFO, invalid, 4); mr60bha2_process();
    CHECK(!context.targets_received);
    emit(0x0A08, invalid, 4); mr60bha2_process();
    CHECK(!context.targets_received);
    reset(); presence(1); distance(100); targets(3); mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_MULTIPLE);
    ++test_number;

    // Corrupt frames cannot refresh old context, and delayed queues retain RX age.
    ready();
    uint8_t h = 1;
    fake_now = 5900; emit(MR60BHA2_TYPE_HUMAN_DETECTION, &h, 1, true);
    mr60bha2_process(); CHECK(context.human_at == 3000U);
    fake_now = 6000; CHECK(view().status == MR60BHA2_VITAL_INVALID);
    ready(); fake_now = 4000; presence(1); distance(100); targets(); rates();
    fake_now = 8000; mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID && context.human_at == 4000U);
    ++test_number;

    ready();
    for(unsigned i = 0; i <= MR60BHA2_RX_BUFFER_SIZE; ++i) mr60bha2_receive_byte(0x55);
    mr60bha2_process();
    CHECK(view().status == MR60BHA2_VITAL_INVALID && !context.human_received);
    sample(4000); sample(5000); sample(6000); sample(7000);
    CHECK(view().status == MR60BHA2_VITAL_REFERENCE);
    ++test_number;

    reset();
    uint32_t start = UINT32_MAX - 1500U;
    for(unsigned i = 0; i <= 3; ++i) sample(start + i*1000U);
    CHECK(view().status == MR60BHA2_VITAL_REFERENCE);
    fake_now = start + 6000U; CHECK(view().status == MR60BHA2_VITAL_INVALID);
    ++test_number;

    // Literal logic-analyzer capture, including trailing bytes of the prior frame.
    reset(); fake_now = 1234;
    const uint8_t capture[] = {0x42, 0x65, 0x01, 0x36, 0xB8, 0x00, 0x02,
                               0x0F, 0x09, 0x74, 0x01, 0x00, 0xFE};
    for(unsigned i = 0; i < sizeof(capture); ++i) mr60bha2_receive_byte(capture[i]);
    mr60bha2_process();
    CHECK(context.human_received && context.human_at == 1234U);
    CHECK(mr60bha2_data.human_detected == 1U);
    CHECK(view().status == MR60BHA2_VITAL_INVALID); // No rate/target/distance yet.
    uint8_t one_byte = 0;
    emit(MR60BHA2_TYPE_HUMAN_DETECTION, &one_byte, 1); mr60bha2_process();
    CHECK(context.human_received && view().status == MR60BHA2_VITAL_ABSENT);
    one_byte = 1;
    emit(MR60BHA2_TYPE_HUMAN_DETECTION, &one_byte, 1); mr60bha2_process();
    CHECK(context.human_received && mr60bha2_data.human_detected == 1U);
    presence(2); mr60bha2_process();
    CHECK(!context.human_received && view().status == MR60BHA2_VITAL_INVALID);
    uint8_t invalid_size[3] = {1, 0, 0};
    emit(MR60BHA2_TYPE_HUMAN_DETECTION, invalid_size, 3); mr60bha2_process();
    CHECK(!context.human_received);
    ++test_number;

    // Radar status cannot cancel or trigger the independent image alarm.
    reset();
    ei_impulse_result_t fall = {1, {{"fall", 0.95f}}};
    fake_now = 100; flow_process_inference_result(&fall);
    fake_now += HM01B0_INITIAL_FALL_MS; flow_process_inference_result(&fall);
    flow_buzzer_tick(fake_now); CHECK(buzzer && reports == 1);
    presence(0); mr60bha2_process(); flow_buzzer_tick(fake_now);
    CHECK(buzzer && hm01b0_fall_detected);
    fake_now = 100 + HM01B0_FALL_HOLD_MS; flow_process_inference_result(&fall);
    CHECK(hm01b0_level2_alert && reports == 2);
    flow_buzzer_tick(fake_now); CHECK(buzzer);
    fall.bounding_boxes_count = 0; flow_process_inference_result(&fall);
    flow_buzzer_tick(fake_now); CHECK(!buzzer);
    ++test_number;

    // Each level is a distinct event, once per transition, with trigger-frame data.
    ready();
    fall.bounding_boxes_count = 1;
    fake_now = 3000; flow_process_inference_result(&fall);
    memset(ei_img_crop_buffer, 0x11, sizeof(ei_img_crop_buffer));
    for(unsigned i = 4; i <= 6; ++i) { sample(i * 1000U); flow_process_inference_result(&fall); }
    CHECK(fall_queue_calls == 1 && queued_falls[0].level == 1);
    CHECK(queued_falls[0].valid && queued_falls[0].heart == 80 && queued_falls[0].breath == 20);
    CHECK(queued_falls[0].first_pixel == 0x11);
    flow_process_inference_result(&fall); CHECK(fall_queue_calls == 1);
    memset(ei_img_crop_buffer, 0x22, sizeof(ei_img_crop_buffer));
    for(unsigned i = 7; i <= 13; ++i) { sample(i * 1000U); flow_process_inference_result(&fall); }
    CHECK(fall_queue_calls == 2 && queued_falls[1].level == 2);
    CHECK(queued_falls[1].first_pixel == 0x22 && queued_falls[0].first_pixel == 0x11);
    sample(14000); flow_process_inference_result(&fall); CHECK(fall_queue_calls == 2);
    fall.bounding_boxes_count = 0; flow_process_inference_result(&fall);
    fall.bounding_boxes_count = 1;
    fake_now = 15000; flow_process_inference_result(&fall);
    fake_now = 18000; flow_process_inference_result(&fall);
    CHECK(fall_queue_calls == 3 && queued_falls[2].level == 1);
    CHECK(!queued_falls[2].valid);
    fake_now = 25000; flow_process_inference_result(&fall);
    CHECK(fall_queue_calls == 4 && queued_falls[3].level == 2);
    ++test_number;
    // Scene-level state: a second fallen person during L1 does not restart it.
    fall.bounding_boxes_count = 0; flow_process_inference_result(&fall);
    reset();
    fall.bounding_boxes_count = 1;
    fake_now = 100; flow_process_inference_result(&fall);
    fake_now = 100 + HM01B0_INITIAL_FALL_MS; flow_process_inference_result(&fall);
    CHECK(fall_queue_calls == 1 && queued_falls[0].level == 1);
    fall.bounding_boxes[1] = {"fall", 0.99f};
    fall.bounding_boxes_count = 2;
    fake_now += 100; flow_process_inference_result(&fall);
    CHECK(fall_queue_calls == 1);
    fake_now = 100 + HM01B0_FALL_HOLD_MS; flow_process_inference_result(&fall);
    CHECK(fall_queue_calls == 2 && queued_falls[1].level == 2);
    ++test_number;
    // Retry preserves the exact crop/vitals; a full queue cannot hold inference forever.
    reset(); reject_fall_queue = true;
    memset(ei_img_crop_buffer, 0x55, sizeof(ei_img_crop_buffer));
    flow_send_fall_event(1U);
    CHECK(pending_fall_levels == (1U << 1));
    fake_now = 499; flow_retry_fall_events();
    CHECK(pending_fall_levels && fall_queue_calls == 0);
    reject_fall_queue = false; flow_retry_fall_events();
    CHECK(!pending_fall_levels && fall_queue_calls == 1 && queued_falls[0].first_pixel == 0x55);
    ++test_number;
    reject_fall_queue = true; fake_now = UINT32_MAX - 100U;
    flow_send_fall_event(2U);
    fake_now += FALL_QUEUE_RETRY_MS;
    flow_retry_fall_events();
    CHECK(!pending_fall_levels && g_flow_metrics.event_drops == 1);
    ++test_number;
    return 0;
}
