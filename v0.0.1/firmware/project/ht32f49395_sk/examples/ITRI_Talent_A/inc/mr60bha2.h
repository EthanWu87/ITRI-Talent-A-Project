#ifndef __MR60BHA2_H
#define __MR60BHA2_H

#include "ht32f493x5_board.h"

#include <stdint.h>
#include <stdbool.h>

#define MR60BHA2_TYPE_PHASE        			0x0A13
#define MR60BHA2_TYPE_BREATH_RATE  			0x0A14
#define MR60BHA2_TYPE_HEART_RATE   			0x0A15
#define MR60BHA2_TYPE_DISTANCE     			0x0A16
#define MR60BHA2_TYPE_HUMAN_DETECTION		0x0F09

#define MR60BHA2_TYPE_TARGET_INFO       0x0A04
#define MR60BHA2_MAX_TARGETS            3U
#define MR60BHA2_DATA_TIMEOUT_MS        3000U
#define MR60BHA2_MAX_DISTANCE_CM        150.0f

#define MR60BHA2_RX_BUFFER_SIZE    			1024
#define MR60BHA2_MAX_DATA_SIZE     			64

/* MR60BHA2 USART configuration */
#define MR60BHA2_USART_PORT             UART7
#define MR60BHA2_USART_CLK              CRM_UART7_PERIPH_CLOCK

#define MR60BHA2_USART_TX_PIN           GPIO_PINS_7
#define MR60BHA2_USART_TX_GPIO_PORT     GPIOE
#define MR60BHA2_USART_TX_GPIO_CLK      CRM_GPIOE_PERIPH_CLOCK

#define MR60BHA2_USART_RX_PIN           GPIO_PINS_8
#define MR60BHA2_USART_RX_GPIO_PORT     GPIOE
#define MR60BHA2_USART_RX_GPIO_CLK      CRM_GPIOE_PERIPH_CLOCK

#define MR60BHA2_USART_IRQn             UART7_IRQn

typedef enum
{
	MR60BHA2_VITAL_INVALID = 0,
	MR60BHA2_VITAL_ABSENT,
	MR60BHA2_VITAL_MULTIPLE,
	MR60BHA2_VITAL_REFERENCE
} MR60BHA2_VitalStatus_t;

typedef struct
{
	MR60BHA2_VitalStatus_t status;
	float heart_rate;
	float breath_rate;
} MR60BHA2_Vitals_t;

typedef struct
{
	float total_phase;
	float breath_phase;
	float heart_phase;

	float breath_rate;
	float heart_rate;

	uint32_t distance_valid;
	float distance_cm;

	bool phase_updated;
	bool breath_rate_updated;
	bool heart_rate_updated;
	bool distance_updated;
	
	uint8_t human_detected;
	bool human_detected_updated;
} MR60BHA2_Data_t;

#ifdef __cplusplus
extern "C" {
#endif

void mr60bha2_init(void);
void mr60bha2_get_vitals(MR60BHA2_Vitals_t *vitals);
void mr60bha2_receive_byte(uint8_t data);
void mr60bha2_process(void);
void mr60bha2_parse_byte(uint8_t data);
bool mr60bha2_get_human_detected(uint8_t *detected);
bool mr60bha2_get_breath_rate(float *breath_rate);
bool mr60bha2_get_heart_rate(float *heart_rate);
bool mr60bha2_get_distance(float *distance_cm);

#ifdef __cplusplus
}
#endif

#endif
