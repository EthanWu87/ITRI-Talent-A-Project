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

extern uint8_t hm01b0_frame_buffer[HM01B0_IMAGE_SIZE_BYTES];

static uint8_t ei_img_crop_buffer[EI_BUFFER_SIZE];   // crop img form 160*120 to 96*96 (center crop) 

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
        accepted ? "queued" : "dropped: queue full or invalid packet");
}

static void flow_send_vital_signs(void)
{
	MR60BHA2_Vitals_t vitals;
	mr60bha2_get_vitals(&vitals);
	const char *status = "invalid";
	if(vitals.status == MR60BHA2_VITAL_ABSENT) status = "absent";
	else if(vitals.status == MR60BHA2_VITAL_MULTIPLE) status = "multiple";
	else if(vitals.status == MR60BHA2_VITAL_REFERENCE) status = "reference";

	ei_printf("VITALS: heart=");
	if(vitals.status == MR60BHA2_VITAL_REFERENCE)
		ei_printf_float(vitals.heart_rate);
	else
		ei_printf("N/A");
	ei_printf(" breath=");
	if(vitals.status == MR60BHA2_VITAL_REFERENCE)
		ei_printf_float(vitals.breath_rate);
	else
		ei_printf("N/A");
	ei_printf(" status=%s\r\n", status);
}

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
	
	return 0;
}

int ei_main(void)
{
	flow_wifi_init();
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
			mr60bha2_process();
			flow_wifi_process();
			dw1000_process();

			if(g_hm01b0.frame_ready)
			{
				ei_printf("\nStarting inferencing\r\n");
				exint_flag_clear(HM01B0_VSYNC_EXINT_LINE);
				nvic_irq_disable(HM01B0_VSYNC_IRQn);

				g_hm01b0.frame_ready = 0;
				
				hm01b0_effective_crop(&g_hm01b0, ei_img_crop_buffer, EI_TARGET_WIDTH, EI_TARGET_HEIGHT);

				printf("IMG_S");
        for(int i = 0; i < EI_BUFFER_SIZE; i++)
        {
          while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET);
          usart_data_transmit(USART1, ei_img_crop_buffer[i]);
        }
        printf("IMG_E\r\n");
				
				exint_flag_clear(HM01B0_VSYNC_EXINT_LINE);
				nvic_irq_enable(HM01B0_VSYNC_IRQn, 0, 0);
				
				signal_t features_signal;
				features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
				features_signal.get_data = &raw_feature_get_data;
	
				/* run classifier */
				EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
				printf("Run classifier success\n");
				
				if(res != 0)
				{
					return 1;
				}

				mr60bha2_process();
				flow_process_inference_result(&result);
				
				printf("INF_S");   // inference results start header 
				ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
						result.timing.dsp, result.timing.classification, result.timing.anomaly);

				#if EI_CLASSIFIER_OBJECT_DETECTION == 1
					for (uint32_t i = 0; i < result.bounding_boxes_count; i++)
					{
						ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
						ei_printf("  %s (", bb.label);
						ei_printf_float(bb.value);
						ei_printf(") [ x: %u, y: %u, width: %u, height: %u ]\r\n",
							bb.x,
							bb.y,
							bb.width,
							bb.height);
					}

				#else
					for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
						ei_printf("  %s: ", result.classification[i].label);
						ei_printf_float(result.classification[i].value);
						ei_printf("\r\n");
					}
				#endif
				flow_send_vital_signs();
				printf("INF_E\r\n");

				/* Service UWB/TCP after completing the image/inference packet. */
				flow_wifi_process();
				dw1000_process();
			}
		}
	}
}
