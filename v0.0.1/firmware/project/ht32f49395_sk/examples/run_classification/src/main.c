#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"
#include "i2c_application.h"
#include "hm01b0.h"

#define UART_BAUD_RATE    2250000

#define MS_TICK   (system_core_clock / 1000U)

extern int ei_main(void);

hm01b0_t g_hm01b0;

uint8_t hm01b0_frame_buffer[HM01B0_IMAGE_SIZE_BYTES];

/**
  * @brief  config systick and enable interrupt.
  * @param  ticks: number of clock cycles between interrupts
  * @retval 1: configuration successful
	*         0: configuration faild
  */
static uint32_t systick_interrupt_config(uint32_t ticks)
{
  if((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk)
  {
    return (1UL);
  }

  SysTick->LOAD  = (uint32_t)(ticks - 1UL);
  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
  SysTick->VAL   = 0UL;
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

  return (0UL);
}

int main(void)
{	
	system_clock_config();

	/* configure SysTick clock source to AHB clock */
	systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
	systick_interrupt_config(MS_TICK);
	
	ht32_board_init();

	uart_print_init(UART_BAUD_RATE);

	printf("\r\n--- System Starting ---\r\n");

	if(hm01b0_init(&g_hm01b0, hm01b0_frame_buffer, sizeof(hm01b0_frame_buffer)) != 0)
	{
		printf("HM01B0 initialization failed!\r\n");

		while(1)
		{
			
		}
	}

	printf("HM01B0 Model ID: 0x%04X\r\n", g_hm01b0.model_id);
	printf("HM01B0 initialized.\r\n");
	
	ei_main();
	
	while(1)
	{

	}
}
