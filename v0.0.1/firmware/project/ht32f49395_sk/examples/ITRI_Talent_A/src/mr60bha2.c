#include "mr60bha2.h"
#include <string.h>

/* Static variable */
static volatile uint8_t rx_buffer[MR60BHA2_RX_BUFFER_SIZE];

static volatile uint16_t rx_write_index = 0;
static volatile uint16_t rx_read_index = 0;

static MR60BHA2_Data_t mr60bha2_data;

/* Static function */
static void mr60bha2_usart_configuration(void);
static void mr60bha2_reset_parser(void);
static void mr60bha2_handle_frame(void);

static float mr60bha2_bytes_to_float(const uint8_t *data);
static uint32_t mr60bha2_bytes_to_uint32(const uint8_t *data);

/* States of the MR60BHA2 frame parsing finite state machine */
typedef enum
{
	TF_WAIT_SOF = 0,

	TF_ID_H,
	TF_ID_L,

	TF_LEN_H,
	TF_LEN_L,

	TF_TYPE_H,
	TF_TYPE_L,

	TF_HEAD_CKSUM,

	TF_DATA,

	TF_DATA_CKSUM
	
} MR60BHA2_ParserState_t;

/* */
typedef struct
{
	MR60BHA2_ParserState_t state;

	uint16_t id;
	uint16_t len;
	uint16_t type;

	uint8_t header_checksum;
	uint8_t data_checksum;

	uint8_t header_calc;
	uint8_t data_calc;

	uint16_t data_index;

	uint8_t data[MR60BHA2_MAX_DATA_SIZE];
	
} MR60BHA2_Frame_t;


static MR60BHA2_Frame_t frame;

static void mr60bha2_usart_configuration(void)
{
	gpio_init_type gpio_init_struct;

  /* Enable the usart2 and gpio clock */
  crm_periph_clock_enable(MR60BHA2_USART_CLK, TRUE);
  crm_periph_clock_enable(MR60BHA2_USART_TX_GPIO_CLK, TRUE);
	
	/* Configure the usart2 tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = MR60BHA2_USART_TX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(MR60BHA2_USART_TX_GPIO_PORT, &gpio_init_struct);
	
  /* Configure the usart2 rx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = MR60BHA2_USART_RX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(MR60BHA2_USART_RX_GPIO_PORT, &gpio_init_struct);

  /* Config usart nvic interrupt */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(MR60BHA2_USART_IRQn, 0, 0);

  /* Configure usart2 param */
  usart_init(MR60BHA2_USART_PORT, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(MR60BHA2_USART_PORT, TRUE);
  usart_receiver_enable(MR60BHA2_USART_PORT, TRUE);

  /* Enable usart2 receive interrupt */
  usart_interrupt_enable(MR60BHA2_USART_PORT, USART_RDBF_INT, TRUE);
  usart_enable(MR60BHA2_USART_PORT, TRUE);
}

static void mr60bha2_reset_parser(void)
{
	frame.state = TF_WAIT_SOF;

	frame.id = 0;
	frame.len = 0;
	frame.type = 0;

	frame.header_checksum = 0;
	frame.data_checksum = 0;

	frame.header_calc = 0;
	frame.data_calc = 0;

	frame.data_index = 0;
}

/**
  * @brief  Parse mr60bha packets (Finite State Machine). 
  * @param  The mr60bha output data (byte).
  * @retval none
  */
void mr60bha2_parse_byte(uint8_t byte)
{
	switch(frame.state)
	{
		case TF_WAIT_SOF:
		{
			
			if(byte == 0x01)  // mr60bha2 SOF value
			{
				frame.header_calc = byte;
				frame.state = TF_ID_H;
			}

			break;
		}
			
		case TF_ID_H:
		{
			frame.id = ((uint16_t)byte << 8);

			frame.header_calc ^= byte;

			frame.state = TF_ID_L;

			break;
    }
		
		case TF_ID_L:
		{	
			frame.id |= byte;

			frame.header_calc ^= byte;

			frame.state = TF_LEN_H;

			break;
    }
		
		case TF_LEN_H:
		{	
			frame.len = ((uint16_t)byte << 8);

			frame.header_calc ^= byte;

			frame.state = TF_LEN_L;

			break;
    }

		case TF_LEN_L:
		{	
			frame.len |= byte;

			frame.header_calc ^= byte;

			if(frame.len > MR60BHA2_MAX_DATA_SIZE)
			{
				mr60bha2_reset_parser();
				break;
			}

			frame.state = TF_TYPE_H;

			break;
    }
		
		case TF_TYPE_H:
		{	
			frame.type = ((uint16_t)byte << 8);

			frame.header_calc ^= byte;

			frame.state = TF_TYPE_L;

			break;
    }
		
		case TF_TYPE_L:
		{	
			frame.type |= byte;

			frame.header_calc ^= byte;

			frame.state = TF_HEAD_CKSUM;

			break;
    }
		
		case TF_HEAD_CKSUM:
		{	
			frame.header_checksum = byte;

			frame.header_calc = ~frame.header_calc;

			if(frame.header_checksum != frame.header_calc)
			{
				mr60bha2_reset_parser();
				break;
			}

			frame.data_index = 0;
			frame.data_calc = 0;

			if(frame.len == 0)
			{
				mr60bha2_reset_parser();
			}
			else
			{
				frame.state = TF_DATA;
			}

			break;
    }
		
		case TF_DATA:
    {
			frame.data[frame.data_index] = byte;

			frame.data_calc ^= byte;

			frame.data_index++;

			if(frame.data_index >= frame.len)
			{
				frame.state = TF_DATA_CKSUM;
			}

			break;
    }
		
		case TF_DATA_CKSUM:
    {
			frame.data_checksum = byte;

			frame.data_calc = ~frame.data_calc;

			if(frame.data_checksum == frame.data_calc)
			{
				mr60bha2_handle_frame();
			}

			mr60bha2_reset_parser();

			break;
    }
		
		default:
    {
			mr60bha2_reset_parser();

			break;
		}
	}
}

/**
  * @brief  Process a validated MR60BHA2 frame and update data by message type.
  * @param  none
  * @retval none
  */
static void mr60bha2_handle_frame(void)
{
	switch(frame.type)
	{
		case MR60BHA2_TYPE_PHASE:
		{
			if(frame.len >= 12)
			{
				mr60bha2_data.total_phase = mr60bha2_bytes_to_float(&frame.data[0]);
				
				mr60bha2_data.breath_phase = mr60bha2_bytes_to_float(&frame.data[4]);
				
				mr60bha2_data.heart_phase = mr60bha2_bytes_to_float(&frame.data[8]);
				
				mr60bha2_data.phase_updated = true;
			}
			
			break;
    }
		
		case MR60BHA2_TYPE_BREATH_RATE:
    {
			if(frame.len >= 4)
			{
				mr60bha2_data.breath_rate = mr60bha2_bytes_to_float(&frame.data[0]);

				mr60bha2_data.breath_rate_updated = true;
			}

			break;
    }
		
		case MR60BHA2_TYPE_HEART_RATE:
	  {
			if(frame.len >= 4)
			{
				mr60bha2_data.heart_rate = mr60bha2_bytes_to_float(&frame.data[0]);

				mr60bha2_data.heart_rate_updated = true;
			}

			break;
    }
		
		case MR60BHA2_TYPE_DISTANCE:
		{	
			if(frame.len >= 8)
			{
				mr60bha2_data.distance_valid = mr60bha2_bytes_to_uint32(&frame.data[0]);

				mr60bha2_data.distance_cm = mr60bha2_bytes_to_float(&frame.data[4]);

				mr60bha2_data.distance_updated = true;
			}
			
			break;
		}
		
		case MR60BHA2_TYPE_HUMAN_DETECTION:
    {
			if(frame.len >= 1)
			{
				mr60bha2_data.human_detected = (frame.data[0] != 0U) ? 1U : 0U;
				mr60bha2_data.human_detected_updated = true;
			}

			break;
    }
		
		default:
		{
			break;
		}
	}
}

static float mr60bha2_bytes_to_float(const uint8_t *data)
{
	float value;

	memcpy(&value, data, sizeof(value));

	return value;
}

static uint32_t mr60bha2_bytes_to_uint32(const uint8_t *data)
{
	return ((uint32_t)data[3]) | ((uint32_t)data[2] << 8) | ((uint32_t)data[1] << 16) | ((uint32_t)data[0] << 24);
}

MR60BHA2_Data_t *mr60bha_get_data(void)
{
	return &mr60bha2_data;
}

void mr60bha2_init(void)
{
	memset(&mr60bha2_data, 0, sizeof(mr60bha2_data));
	memset(&frame, 0, sizeof(frame));

	rx_write_index = 0;
	rx_read_index = 0;

	mr60bha2_reset_parser();
	mr60bha2_usart_configuration();
}

void mr60bha2_receive_byte(uint8_t data)
{
	uint16_t next = (rx_write_index + 1) % MR60BHA2_RX_BUFFER_SIZE;

	if(next != rx_read_index)
	{
		rx_buffer[rx_write_index] = data;
		rx_write_index = next;
	}
}

void mr60bha2_process(void)
{
	uint8_t data;

	while(rx_read_index != rx_write_index)
	{
		data = rx_buffer[rx_read_index];

		rx_read_index = (rx_read_index + 1) % MR60BHA2_RX_BUFFER_SIZE;

		mr60bha2_parse_byte(data);
	}
}

bool mr60bha2_get_human_detected(uint8_t *detected)
{
    if(detected == NULL || !mr60bha2_data.human_detected_updated)
    {
        return false;
    }

    *detected = mr60bha2_data.human_detected;
    mr60bha2_data.human_detected_updated = false;
    return true;
}

bool mr60bha2_get_breath_rate(float *breath_rate)
{
	bool updated;

	if(breath_rate == NULL)
	{
		return false;
	}

	*breath_rate = mr60bha2_data.breath_rate;
	updated = mr60bha2_data.breath_rate_updated;
	mr60bha2_data.breath_rate_updated = false;
	return updated;
}

bool mr60bha2_get_heart_rate(float *heart_rate)
{
    bool updated;

    if(heart_rate == NULL)
    {
        return false;
    }

    *heart_rate = mr60bha2_data.heart_rate;
    updated = mr60bha2_data.heart_rate_updated;
    mr60bha2_data.heart_rate_updated = false;
    return updated;
}

bool mr60bha2_get_distance(float *distance_cm)
{
    bool updated;

    if(distance_cm == NULL)
    {
        return false;
    }

    *distance_cm = mr60bha2_data.distance_cm;
    updated = mr60bha2_data.distance_updated;
    mr60bha2_data.distance_updated = false;
    return updated;
}
