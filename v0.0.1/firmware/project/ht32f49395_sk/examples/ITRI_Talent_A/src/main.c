#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"
#include "BMC81M001.h"
#include "hm01b0.h"
#include "mr60bha2.h"

#define UART_BAUD_RATE    2250000

hm01b0_t g_hm01b0;

uint8_t hm01b0_frame_buffer[HM01B0_IMAGE_SIZE_BYTES];

int main(void)
{
  system_clock_config();

  ht32_board_init();
	
	uart_print_init(UART_BAUD_RATE);
	
  BMC81M001_Init(BMC81M001_baudRate);
	
	
	/* Beep initialization */
	/*
	gpio_init_type gpio_init_struct;

  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
	
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_1;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOB, &gpio_init_struct);
	*/
	
  if(BMC81M001_sendATCommand("AT\r\n", 1) == BMC81M001_SEND_SUCCESS)
  {
		printf("success\r\n");
	}
  else
  {
		printf("fail\r\n");
  }
	
  while(1)
  {
		
  }
}
