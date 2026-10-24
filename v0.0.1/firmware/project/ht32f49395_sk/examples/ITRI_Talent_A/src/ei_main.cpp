#include "ei_main.h"                  
#include "ht32f493x5_board.h"          
#include "ht32f493x5_clock.h"
#include "ht32f493x5_conf.h"
#include "ht32f493x5_int.h"
#include "ei_run_classifier.h"         
#include "edge-impulse-sdk/porting/ei_classifier_porting.h" 
#include "math.h"
#include "hm01b0.h"
#include "mr60bha2.h"
#include "bmc81m001.h"
#include "dw1000.h"
#include "flow_metrics.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define EI_TARGET_WIDTH  96
#define EI_TARGET_HEIGHT 96
#define EI_BUFFER_SIZE 9216

#define HM01B0_INITIAL_FALL_MS 3000U
#define HM01B0_FALL_HOLD_MS 10000U
#define FALL_CONFIDENCE 0.80f
#define BUZZER_BEEP_INTERVAL_MS 1000U
#define BUZZER_BEEP_LENGTH_MS 100U
#define FALL_QUEUE_RETRY_MS 500U
#define INFERENCE_RETRY_LIMIT 3U
#ifndef FLOW_STATS_ENABLE
#define FLOW_STATS_ENABLE 1
#endif
#ifndef FLOW_VERBOSE_PREDICTIONS
#define FLOW_VERBOSE_PREDICTIONS 0
#endif

extern uint8_t hm01b0_frame_buffer[HM01B0_IMAGE_SIZE_BYTES];

static uint8_t ei_img_crop_buffer[EI_BUFFER_SIZE];   // crop img form 160*120 to 96*96 (center crop) 

// BEGIN IMAGE UART DMA
/* USART1 TX owns DMA1 Channel1; camera capture uses DMA1 Channel2.
 * Main-context only: do not print to USART1 from an interrupt.
 * The crop buffer is read-only until flow_image_uart_wait() finishes.
 */
#define IMAGE_UART_DMA_TIMEOUT_MS 1000U
static bool image_tx_active;
static bool image_tx_success = true;
static uint32_t image_tx_started;

static void image_uart_dma_init(void)
{
  dma_init_type config;
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  dma_reset(DMA1_CHANNEL1);
  dma_flexible_config(DMA1, FLEX_CHANNEL1, DMA_FLEXIBLE_UART1_TX);
  dma_default_para_init(&config);
  config.peripheral_base_addr = (uint32_t)&USART1->dt;
  config.memory_base_addr = (uint32_t)ei_img_crop_buffer;
  config.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  config.buffer_size = EI_BUFFER_SIZE;
  config.peripheral_inc_enable = FALSE;
  config.memory_inc_enable = TRUE;
  config.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  config.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  config.loop_mode_enable = FALSE;
  config.priority = DMA_PRIORITY_LOW;
  dma_init(DMA1_CHANNEL1, &config);
}

static void image_uart_dma_start(void)
{
  flow_image_uart_wait();
  printf("IMG_S");
  image_tx_success = true;
  image_tx_active = true;
  image_tx_started = SysTick_GetTick();

  /* Finish IMG_S before DMA takes ownership of the USART data register. */
  while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
  {
    if((SysTick_GetTick() - image_tx_started) >= IMAGE_UART_DMA_TIMEOUT_MS)
    {
      image_tx_success = false;
      return;
    }
  }
  dma_flag_clear(DMA1_GL1_FLAG);
  DMA1_CHANNEL1->maddr = (uint32_t)ei_img_crop_buffer;
  dma_data_number_set(DMA1_CHANNEL1, EI_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);
  usart_dma_transmitter_enable(USART1, TRUE);
}

uint8_t flow_image_uart_wait(void)
{
  if(!image_tx_active) return image_tx_success ? 1U : 0U;

  /* Called after inference, or before an EI diagnostic needs USART1. */
  while(image_tx_success && dma_flag_get(DMA1_FDT1_FLAG) == RESET)
  {
    if(dma_flag_get(DMA1_DTERR1_FLAG) != RESET ||
       (SysTick_GetTick() - image_tx_started) >= IMAGE_UART_DMA_TIMEOUT_MS)
    {
      image_tx_success = false;
      break;
    }
  }
  usart_dma_transmitter_enable(USART1, FALSE);
  dma_channel_enable(DMA1_CHANNEL1, FALSE);
  dma_flag_clear(DMA1_GL1_FLAG);

  /* DMA completion precedes the last byte leaving the UART shift register. */
  uint32_t drain_started = SysTick_GetTick();
  while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
  {
    if((SysTick_GetTick() - drain_started) >= IMAGE_UART_DMA_TIMEOUT_MS)
    {
      image_tx_success = false;
      break;
    }
  }
  image_tx_active = false;
  printf("IMG_E\r\n");
  return image_tx_success ? 1U : 0U;
}
// END IMAGE UART DMA

// BEGIN APPLICATION NETWORK PROTOCOL
#define WIFI_CMD_MAX 120U
#define FLOW_FALL_IMAGE_BYTES 9216U
static char command[WIFI_CMD_MAX];
static uint16_t command_n = 0;
static uint8_t command_overflow;
static char pending_command[WIFI_CMD_MAX];
static uint8_t command_ready = 0;
static void flow_wifi_command_byte(uint8_t b)
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
  if (command_n < WIFI_CMD_MAX -1U) command[command_n++] = (char) b;
  else
  {
    command_n = 0;
    command_overflow = 1U;
  }
}

static uint8_t flow_wifi_queue_fall_event(uint8_t level,
                              uint8_t vitals_valid, float heart_rate, float breath_rate,
                              const uint8_t *image, uint32_t image_size)
{
  char header[112], rates[64], footer[20];
  int header_len, footer_len, rates_len;
  BMC81M001_DataSpan parts[3];
  if(image == NULL || image_size != FLOW_FALL_IMAGE_BYTES ||
     (level != 1U && level != 2U))
    return 0U;
  if(vitals_valid && isfinite(heart_rate) && isfinite(breath_rate) &&
     heart_rate > 0.0f && breath_rate > 0.0f)
    rates_len = snprintf(rates, sizeof(rates), "%.2f,%.2f", (double)heart_rate, (double)breath_rate);
  else
    rates_len = snprintf(rates, sizeof(rates), "N/A,N/A");
  if(rates_len < 0 || rates_len >= (int)sizeof(rates))
    return 0U;
  header_len = snprintf(header, sizeof(header), "Node_%c_S,L%u,%s,",
                        FLOW_NODE_ID, (unsigned)level, rates);
  footer_len = snprintf(footer, sizeof(footer), ",Node_%c_E\r\n", FLOW_NODE_ID);
  if(header_len < 0 || header_len >= (int)sizeof(header) ||
     footer_len < 0 || footer_len >= (int)sizeof(footer) ||
     (uint32_t)(header_len + footer_len) + image_size > BMC81M001_PACKET_MAX)
    return 0U;
  parts[0].data = (const uint8_t *)header; parts[0].length = (uint16_t)header_len;
  parts[1].data = image; parts[1].length = (uint16_t)image_size;
  parts[2].data = (const uint8_t *)footer; parts[2].length = (uint16_t)footer_len;
  return BMC81M001_QueuePacket(parts, 3U, level);
}

uint8_t flow_wifi_send_control(const char *kind, unsigned seq, const char *value)
{
  char message[BMC81M001_CONTROL_MAX];
  int length = snprintf(message, sizeof(message), "NODE_%c,%s,%u,%s\n",
                        FLOW_NODE_ID, kind, seq, value);
  if(length <= 0 || length >= (int)sizeof(message)) return 0U;
  return BMC81M001_QueueData((const uint8_t *)message, (uint16_t)length);
}

static void flow_wifi_link_changed(uint8_t connected)
{
  command_ready = 0U;
  command_n = 0U;
  command_overflow = 0U;
  if(connected) flow_wifi_send_control("HELLO", 0U, "ONLINE");
  else dw1000_on_disconnect();
}

static void flow_wifi_packet_sent(uint32_t token)
{
  printf("[FALL TX] level=L%lu sent\r\n", (unsigned long)token);
}

static void flow_wifi_init(void)
{
  static const BMC81M001_TcpCallbacks callbacks = {
    flow_wifi_command_byte, flow_wifi_link_changed, flow_wifi_packet_sent
  };
  command_ready = 0U;
  command_n = 0U;
  command_overflow = 0U;
  BMC81M001_TcpInit(&callbacks);
}

static void flow_wifi_process(void)
{
  BMC81M001_Process();
  if(!BMC81M001_IsConnected()) return;
  /* Reserve capacity for all replies from one PC command. */
  if(command_ready && !dw1000_is_busy() && BMC81M001_ControlSlotsFree() >= 4U)
  {
    char local[WIFI_CMD_MAX];
    strcpy(local, pending_command);
    command_ready = 0U;
    dw1000_execute_command(local);
  }
}
// END APPLICATION NETWORK PROTOCOL

// Radar vital signs are reference-only; alarms come from image fall detection.
static volatile bool hm01b0_fall_detected;
static volatile bool hm01b0_level2_alert;
static volatile uint32_t hm01b0_fall_since;
static volatile uint32_t buzzer_started;
static volatile uint32_t buzzer_last_beep;
/* While pending, main retains the crop and services communications for <=500 ms.
 * No extra 9 KB copy and no substitution of a later image into an old event.
 */
static uint8_t pending_fall_levels;
static uint32_t pending_fall_since;
static MR60BHA2_Vitals_t pending_fall_vitals;

static void flow_send_report(const char *message)
{
	printf("REPORT: %s\r\n", message);
}

static void flow_send_fall_event(uint8_t level)
{
    MR60BHA2_Vitals_t vitals;
    mr60bha2_get_vitals(&vitals);
    uint8_t accepted = flow_wifi_queue_fall_event(level,
        vitals.status == MR60BHA2_VITAL_REFERENCE, vitals.heart_rate, vitals.breath_rate,
        ei_img_crop_buffer, EI_BUFFER_SIZE);
    printf("[FALL EVENT] level=L%u %s\r\n",
        (unsigned)level,
        accepted ? "queued" : "pending: queue full");
    if(!accepted)
    {
      if(!pending_fall_levels) pending_fall_since = SysTick_GetTick();
      pending_fall_levels |= (uint8_t)(1U << level);
      pending_fall_vitals = vitals;
    }
}

static void flow_retry_fall_events(void)
{
  if(!pending_fall_levels) return;
  for(uint8_t level = 1U; level <= 2U; ++level)
  {
    uint8_t mask = (uint8_t)(1U << level);
    if(!(pending_fall_levels & mask)) continue;
    if(flow_wifi_queue_fall_event(level,
        pending_fall_vitals.status == MR60BHA2_VITAL_REFERENCE,
        pending_fall_vitals.heart_rate, pending_fall_vitals.breath_rate,
        ei_img_crop_buffer, EI_BUFFER_SIZE))
    {
      pending_fall_levels &= (uint8_t)~mask;
      printf("[FALL EVENT] level=L%u queued after retry\r\n", (unsigned)level);
    }
    else if((uint32_t)(SysTick_GetTick() - pending_fall_since) >= FALL_QUEUE_RETRY_MS)
    {
      pending_fall_levels &= (uint8_t)~mask;
      ++g_flow_metrics.event_drops;
      printf("[FALL EVENT] level=L%u dropped: retry expired\r\n", (unsigned)level);
    }
  }
}

static void flow_send_vital_signs(void)
{
	MR60BHA2_Vitals_t vitals;
	mr60bha2_get_vitals(&vitals);
	const char *status = "invalid";
	if(vitals.status == MR60BHA2_VITAL_ABSENT) status = "absent";
	else if(vitals.status == MR60BHA2_VITAL_MULTIPLE) status = "multiple";
	else if(vitals.status == MR60BHA2_VITAL_REFERENCE) status = "reference";

	if(vitals.status == MR60BHA2_VITAL_REFERENCE)
		ei_printf("VITALS: heart=%.2f breath=%.2f status=%s\r\n",
		          (double)vitals.heart_rate, (double)vitals.breath_rate, status);
	else
		ei_printf("VITALS: heart=N/A breath=N/A status=%s\r\n", status);
}

#if !defined(FLOW_LIGHTWEIGHT_TEST)
static void flow_print_predictions(const ei_impulse_result_t *result)
{
  char output[256];
  int length;
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
  const ei_impulse_result_bounding_box_t *best = NULL;
  for(uint32_t i = 0; i < result->bounding_boxes_count; ++i)
  {
    if(result->bounding_boxes[i].value > 0.0f &&
       (best == NULL || result->bounding_boxes[i].value > best->value))
      best = &result->bounding_boxes[i];
  }
  if(best != NULL)
    length = snprintf(output, sizeof(output),
      "PRED,%s,%.3f,%u,%u,%u,%u,dsp=%d,cls=%d,anom=%d\r\n",
      best->label, (double)best->value, (unsigned)best->x,
      (unsigned)best->y, (unsigned)best->width, (unsigned)best->height,
      result->timing.dsp, result->timing.classification, result->timing.anomaly);
  else
    length = snprintf(output, sizeof(output),
      "PRED,NONE,dsp=%d,cls=%d,anom=%d\r\n",
      result->timing.dsp, result->timing.classification, result->timing.anomaly);
#else
  uint16_t best_index = 0U;
  for(uint16_t i = 1U; i < EI_CLASSIFIER_LABEL_COUNT; ++i)
    if(result->classification[i].value > result->classification[best_index].value)
      best_index = i;
  length = snprintf(output, sizeof(output),
    "PRED,%s,%.3f,dsp=%d,cls=%d,anom=%d\r\n",
    result->classification[best_index].label,
    (double)result->classification[best_index].value,
    result->timing.dsp, result->timing.classification, result->timing.anomaly);
#endif
  if(length > 0)
  {
    if(length >= (int)sizeof(output)) length = (int)sizeof(output) - 1;
    ei_printf("%.*s", length, output);
  }

#if FLOW_VERBOSE_PREDICTIONS
  ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.):\r\n",
    result->timing.dsp, result->timing.classification, result->timing.anomaly);
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
  for(uint32_t i = 0; i < result->bounding_boxes_count; ++i)
  {
    const ei_impulse_result_bounding_box_t *bb = &result->bounding_boxes[i];
    if(bb->value > 0.0f)
      ei_printf("  %s (%.5f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
        bb->label, (double)bb->value, (unsigned)bb->x, (unsigned)bb->y,
        (unsigned)bb->width, (unsigned)bb->height);
  }
#else
  for(uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i)
    ei_printf("  %s: %.5f\r\n", result->classification[i].label,
              (double)result->classification[i].value);
#endif
#endif
}

static void flow_recover_capture(void)
{
  nvic_irq_disable(HM01B0_VSYNC_IRQn);
  dma_channel_enable(HM01B0_DMA_CHANNEL, FALSE);
  dma_flag_clear(HM01B0_DMA_FDT_FLAG);
  g_hm01b0.frame_ready = 0U;
  exint_flag_clear(HM01B0_VSYNC_EXINT_LINE);
  nvic_irq_enable(HM01B0_VSYNC_IRQn, 0, 0);
}
#endif

extern "C" void flow_buzzer_tick(uint32_t now)
{
	if(!hm01b0_fall_detected)
	{
		gpio_bits_reset(GPIOB, GPIO_PINS_1);
		buzzer_started = 0U;
		buzzer_last_beep = 0U;
		return;
	}

	if(hm01b0_level2_alert)
	{
		gpio_bits_set(GPIOB, GPIO_PINS_1);
		return;
	}

	if(buzzer_started != 0U && (now - buzzer_started) < BUZZER_BEEP_LENGTH_MS)
	{
		gpio_bits_set(GPIOB, GPIO_PINS_1);
		return;
	}

	gpio_bits_reset(GPIOB, GPIO_PINS_1);
	buzzer_started = 0U;

	if(buzzer_last_beep == 0U || (now - buzzer_last_beep) >= BUZZER_BEEP_INTERVAL_MS)
	{
		buzzer_started = now;
		buzzer_last_beep = now;
		gpio_bits_set(GPIOB, GPIO_PINS_1);
	}
}

static void flow_process_inference_result(const ei_impulse_result_t *result)
{
	static bool fall_tracking;
	float fall_confidence = 0.0f;
	uint32_t now = SysTick_GetTick();

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
	for(uint32_t i = 0; i < result->bounding_boxes_count; i++)
	{
		if(strstr(result->bounding_boxes[i].label, "fall") != NULL &&
			result->bounding_boxes[i].value > fall_confidence)
		{
			fall_confidence = result->bounding_boxes[i].value;
		}
	}
#else
	for(uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
	{
		if(strstr(result->classification[i].label, "fall") != NULL &&
			result->classification[i].value > fall_confidence)
		{
			fall_confidence = result->classification[i].value;
		}
	}
#endif

	if(fall_confidence >= FALL_CONFIDENCE)
	{
		if(!fall_tracking)
		{
			fall_tracking = true;
			hm01b0_fall_since = now;
		}
		if(!hm01b0_fall_detected &&
			(now - hm01b0_fall_since) >= HM01B0_INITIAL_FALL_MS)
		{
			hm01b0_fall_detected = true;
			flow_send_report("level 1 fall image alert");
			flow_send_fall_event(1U);
		}
		if((now - hm01b0_fall_since) >= HM01B0_FALL_HOLD_MS && !hm01b0_level2_alert)
		{
			hm01b0_level2_alert = true;
			flow_send_report("level 2 fall image alert");
			flow_send_fall_event(2U);
		}
	}
	else
	{
		fall_tracking = false;
		hm01b0_fall_detected = false;
		hm01b0_level2_alert = false;
		hm01b0_fall_since = 0U;
	}
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr)
{
	uint8_t *src = &ei_img_crop_buffer[offset];
	
	for(size_t i = 0; i < length; i++)
	{
		uint8_t gray = src[i];
		uint32_t rgb = (gray << 16) | (gray << 8) | gray;
		out_ptr[i] = (float)rgb;
	}
	flow_service_background();
	
	return 0;
}

static bool background_ready;

void flow_service_background(void)
{
  static bool servicing;
  static uint32_t last_service, last_attempt;
  uint32_t now = SysTick_GetTick();
  if(!background_ready || servicing || (uint32_t)(now - last_attempt) < 5U) return;
  last_attempt = now;
  servicing = true;
  mr60bha2_process();
  /* These services can print. Only run them after the raw image is complete. */
  if(image_tx_active && dma_flag_get(DMA1_FDT1_FLAG) == RESET &&
     dma_flag_get(DMA1_DTERR1_FLAG) == RESET &&
     (uint32_t)(now - image_tx_started) < IMAGE_UART_DMA_TIMEOUT_MS)
  {
    servicing = false;
    return;
  }
  flow_image_uart_wait();
  uint32_t gap = now - last_service;
  if(last_service && gap > g_flow_metrics.service_gap_ms) g_flow_metrics.service_gap_ms = gap;
  last_service = now;
  flow_wifi_process();
  dw1000_process();
  servicing = false;
}

static void flow_report_performance(uint32_t crop, uint32_t inference, uint32_t wait, uint32_t text)
{
#if FLOW_STATS_ENABLE
  static uint32_t since, frames, crop_us, inference_us, wait_us, text_us;
  static bool started;
  uint32_t now = SysTick_GetTick();
  if(!started) { since = now; started = true; return; }
  ++frames;
  crop_us += crop; inference_us += inference; wait_us += wait; text_us += text;
  if((uint32_t)(now - since) < 1000U) return;
  ei_printf("[PERF] frames=%lu interval_ms=%lu crop_us=%lu infer_us=%lu wait_us=%lu text_us=%lu\r\n",
    (unsigned long)frames, (unsigned long)(now - since),
    (unsigned long)(crop_us / frames), (unsigned long)(inference_us / frames),
    (unsigned long)(wait_us / frames), (unsigned long)(text_us / frames));
  ei_printf("[HEALTH] gap_ms=%lu infer_block_ms=%lu dma_wait_ms=%lu text_block_ms=%lu rx_peak=%lu/%lu/%lu rx_drop=%lu/%lu/%lu infer_err=%lu event_drop=%lu\r\n",
    (unsigned long)g_flow_metrics.service_gap_ms,
    (unsigned long)g_flow_metrics.inference_block_ms,
    (unsigned long)g_flow_metrics.dma_wait_ms,
    (unsigned long)g_flow_metrics.text_block_ms,
    (unsigned long)g_flow_metrics.radar_rx_peak, (unsigned long)g_flow_metrics.wifi_rx_peak, (unsigned long)g_flow_metrics.uwb_rx_peak,
    (unsigned long)g_flow_metrics.radar_rx_drops, (unsigned long)g_flow_metrics.wifi_rx_drops, (unsigned long)g_flow_metrics.uwb_rx_drops,
    (unsigned long)g_flow_metrics.inference_errors, (unsigned long)g_flow_metrics.event_drops);
  since = now; frames = crop_us = inference_us = wait_us = text_us = 0U;
#else
  (void)crop; (void)inference; (void)wait; (void)text;
#endif
}

int ei_main(void)
{
	image_uart_dma_init();
	flow_wifi_init();
	background_ready = true;
	uint32_t inference_retry_at = 0U;
	uint32_t inference_failures = 0U;
	ei_impulse_result_t result = {0};

	while(1)
	{
		ei_printf("Edge Impulse standalone inferencing (ht32f493x5)\n");

		/* check the DSP input size */
		if(sizeof(ei_img_crop_buffer) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
		{
			ei_printf("The size of your buffer is not correct. Expected %d items, but had %u\n",
											EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(ei_img_crop_buffer));
			return 1;          
		}

		while(1)
		{
			flow_service_background();
			flow_retry_fall_events();
			if(pending_fall_levels) continue;
			/* Some SDK error paths retain allocations. Bound retries instead of
			 * repeatedly allocating after a persistent model/heap fault.
			 */
			if(inference_failures >= INFERENCE_RETRY_LIMIT) continue;
			if(inference_failures && (int32_t)(SysTick_GetTick() - inference_retry_at) < 0) continue;

			if(g_hm01b0.frame_ready)
			{
				uint64_t crop_started = SysTick_GetTimeUs();
				exint_flag_clear(HM01B0_VSYNC_EXINT_LINE);
				nvic_irq_disable(HM01B0_VSYNC_IRQn);

				g_hm01b0.frame_ready = 0;
				
				hm01b0_effective_crop(&g_hm01b0, ei_img_crop_buffer, EI_TARGET_WIDTH, EI_TARGET_HEIGHT);

				/* Camera capture writes its own buffer; the crop is now stable. */
				exint_flag_clear(HM01B0_VSYNC_EXINT_LINE);
				nvic_irq_enable(HM01B0_VSYNC_IRQn, 0, 0);
				uint32_t crop_us = (uint32_t)(SysTick_GetTimeUs() - crop_started);
				
				signal_t features_signal;
				features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
				features_signal.get_data = &raw_feature_get_data;
	
				/* DMA and inference both read the crop buffer concurrently.
				 * Finish the image before any subsequent printf or next crop.
				 */
				image_uart_dma_start();
				uint64_t inference_started = SysTick_GetTimeUs();
				EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
				uint32_t inference_us = (uint32_t)(SysTick_GetTimeUs() - inference_started);
				uint64_t wait_started = SysTick_GetTimeUs();
				if(!flow_image_uart_wait())
				{
					printf("Image UART DMA transfer failed\r\n");
				}
				uint32_t wait_us = (uint32_t)(SysTick_GetTimeUs() - wait_started);
				
				if(res != 0)
				{
					printf("Run classifier failed: %d\r\n", (int)res);
					++g_flow_metrics.inference_errors;
					++inference_failures;
					memset(&result, 0, sizeof(result));
					flow_recover_capture();
					inference_retry_at = SysTick_GetTick() + inference_failures * 250U;
					if(inference_failures >= INFERENCE_RETRY_LIMIT)
						printf("[INFERENCE] suspended after 3 failures; capture was recovered; communications remain active\r\n");
					continue;
				}
				inference_failures = 0U;
				uint64_t text_started = SysTick_GetTimeUs();

				mr60bha2_process();
				flow_process_inference_result(&result);
				
				printf("INF_S");   // inference results start header 
				flow_print_predictions(&result);
				flow_send_vital_signs();
				printf("INF_E\r\n");
				uint32_t text_us = (uint32_t)(SysTick_GetTimeUs() - text_started);
				flow_report_performance(crop_us, inference_us, wait_us, text_us);
				if(inference_us / 1000U > g_flow_metrics.inference_block_ms)
					g_flow_metrics.inference_block_ms = inference_us / 1000U;
				if(wait_us / 1000U > g_flow_metrics.dma_wait_ms)
					g_flow_metrics.dma_wait_ms = wait_us / 1000U;
				if(text_us / 1000U > g_flow_metrics.text_block_ms)
					g_flow_metrics.text_block_ms = text_us / 1000U;

				/* Service UWB/TCP after completing the image/inference packet. */
				flow_service_background();
			}
		}
	}
}
