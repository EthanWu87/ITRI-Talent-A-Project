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

#define EI_TARGET_WIDTH  96
#define EI_TARGET_HEIGHT 96
#define EI_BUFFER_SIZE 9216

#define MR60BHA2_BREATH_RATE_MIN 12.0f
#define MR60BHA2_BREATH_RATE_MAX 30.0f
#define MR60BHA2_HEART_RATE_MIN 50.0f
#define MR60BHA2_HEART_RATE_MAX 150.0f
#define MR60BHA2_INITIAL_ABNORMAL_MS 2000U
#define MR60BHA2_CONTINUOUS_ABNORMAL_MS 5000U
#define HM01B0_INITIAL_FALL_MS 2000U
#define HM01B0_FALL_HOLD_MS 5000U
#define FALL_CONFIDENCE 0.80f
#define BUZZER_BEEP_INTERVAL_MS 1000U
#define BUZZER_BEEP_LENGTH_MS 100U

extern uint8_t hm01b0_frame_buffer[HM01B0_IMAGE_SIZE_BYTES];

static uint8_t ei_img_crop_buffer[EI_BUFFER_SIZE];   // crop img form 160*120 to 96*96 (center crop) 

// Latest radar samples shared by alarm processing and UART telemetry.
static float breath_rate;
static float heart_rate;
static bool breath_rate_received;
static bool heart_rate_received;

static volatile bool mr60bha2_abnormal;
static volatile bool mr60bha2_level2_alert;
static volatile bool hm01b0_fall_detected;
static volatile bool hm01b0_level2_alert;
static volatile uint32_t mr60bha2_abnormal_since;
static volatile uint32_t hm01b0_fall_since;
static volatile uint32_t buzzer_started;
static volatile uint32_t buzzer_last_beep;

static bool flow_vital_signs_abnormal(float breath_rate, float heart_rate)
{
	return breath_rate < MR60BHA2_BREATH_RATE_MIN ||
		breath_rate > MR60BHA2_BREATH_RATE_MAX ||
		heart_rate < MR60BHA2_HEART_RATE_MIN ||
		heart_rate > MR60BHA2_HEART_RATE_MAX;
}

static void flow_send_report(const char *message)
{
	printf("REPORT: %s\r\n", message);
}

static void flow_send_vital_signs(void)
{
	ei_printf("VITALS: heart=");
	if(heart_rate_received && isfinite(heart_rate))
	{
		ei_printf_float(heart_rate);
	}
	else
	{
		ei_printf("N/A");
	}
	ei_printf(" breath=");
	if(breath_rate_received && isfinite(breath_rate))
	{
		ei_printf_float(breath_rate);
	}
	else
	{
		ei_printf("N/A");
	}
	ei_printf("\r\n");
}

extern "C" void flow_buzzer_tick(uint32_t now)
{
	if(!mr60bha2_abnormal && !hm01b0_fall_detected)
	{
		gpio_bits_reset(GPIOB, GPIO_PINS_1);
		buzzer_started = 0U;
		buzzer_last_beep = 0U;
		return;
	}

	if(mr60bha2_level2_alert || hm01b0_level2_alert)
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

static void flow_process_mr60bha2(void)
{
	static bool abnormal_tracking;
	static uint8_t human_detected;
	bool breath_updated;
	bool heart_updated;
	uint32_t now = SysTick_GetTick();

	mr60bha2_process();
	mr60bha2_get_human_detected(&human_detected);

	breath_updated = mr60bha2_get_breath_rate(&breath_rate);
	heart_updated = mr60bha2_get_heart_rate(&heart_rate);
	breath_rate_received = breath_rate_received || breath_updated;
	heart_rate_received = heart_rate_received || heart_updated;

	// Unknown presence at startup is treated as absent until a human frame arrives.
	// Handle absence even when no new vital-sign sample accompanies it.
	if(human_detected == 0U)
	{
		abnormal_tracking = false;
		mr60bha2_abnormal = false;
		mr60bha2_level2_alert = false;
		mr60bha2_abnormal_since = 0U;
		return;
	}

	if(!breath_updated && !heart_updated)
	{
		return;
	}

	if(flow_vital_signs_abnormal(breath_rate, heart_rate))
	{
		if(!abnormal_tracking)
		{
			abnormal_tracking = true;
			mr60bha2_abnormal_since = now;
		}
		if(!mr60bha2_abnormal &&
			(now - mr60bha2_abnormal_since) >= MR60BHA2_INITIAL_ABNORMAL_MS)
		{
			mr60bha2_abnormal = true;
			flow_send_report("level 1 vital signs alert");
		}
		if((now - mr60bha2_abnormal_since) >= MR60BHA2_CONTINUOUS_ABNORMAL_MS &&
				!mr60bha2_level2_alert)
		{
			mr60bha2_level2_alert = true;
			flow_send_report("level 2 continuous vital signs alert");
		}
	}
	else
	{
		abnormal_tracking = false;
		mr60bha2_abnormal = false;
		mr60bha2_level2_alert = false;
		mr60bha2_abnormal_since = 0U;
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
		}
		if((now - hm01b0_fall_since) >= HM01B0_FALL_HOLD_MS && !hm01b0_level2_alert)
		{
			hm01b0_level2_alert = true;
			flow_send_report("level 2 fall image alert");
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
			flow_process_mr60bha2();
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
				dw1000_process();
			}
		}
	}
}
