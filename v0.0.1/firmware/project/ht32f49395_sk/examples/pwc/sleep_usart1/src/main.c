/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 142         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   main program
 *************************************************************************************************************
 * @attention
 *
 * Firmware Disclaimer Information
 *
 * 1. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, which is supplied by Holtek Semiconductor Inc., (hereinafter referred to as "HOLTEK") is the
 *    proprietary and confidential intellectual property of HOLTEK, and is protected by copyright law and
 *    other intellectual property laws.
 *
 * 2. The customer hereby acknowledges and agrees that the program technical documentation, including the
 *    code, is confidential information belonging to HOLTEK, and must not be disclosed to any third parties
 *    other than HOLTEK and the customer.
 *
 * 3. The program technical documentation, including the code, is provided "as is" and for customer reference
 *    only. After delivery by HOLTEK, the customer shall use the program technical documentation, including
 *    the code, at their own risk. HOLTEK disclaims any expressed, implied or statutory warranties, including
 *    the warranties of merchantability, satisfactory quality and fitness for a particular purpose.
 *
 * <h2><center>Copyright (C) Holtek Semiconductor Inc. All rights reserved</center></h2>
 ************************************************************************************************************/

#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_PWC_sleep_usart1 PWC_sleep_usart1
  * @{
  */

/**
  * @brief  usart1 configuration.
  * @param  none
  * @retval none
  */
void usart1_config(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

  /* enable the uart1 and gpio clock */
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the uart1 tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
  gpio_init_struct.gpio_pins           = GPIO_PINS_9;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pull           = GPIO_PULL_UP;
  gpio_init_struct.gpio_pins           = GPIO_PINS_10;
  gpio_init(GPIOA, &gpio_init_struct);

  /* configure uart param */
  nvic_irq_enable(USART1_IRQn, 0, 0);

  usart_init(USART1, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_parity_selection_config(USART1, USART_PARITY_NONE);
  usart_transmitter_enable(USART1, TRUE);
  usart_receiver_enable(USART1, TRUE);
  usart_hardware_flow_control_set(USART1,USART_HARDWARE_FLOW_NONE);
  usart_interrupt_enable(USART1, USART_RDBF_INT, TRUE);
  usart_enable(USART1, TRUE);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  __IO uint32_t index = 0;
  __IO uint32_t systick_index = 0;

  /* congfig the system clock */
  system_clock_config();

  /* init ht32 starter kit */
  ht32_board_init();

  /* config priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  /* turn on the led light */
  ht32_led_on(LED2);
  ht32_led_on(LED3);
  ht32_led_on(LED4);

  /* enable pwc clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

  /* config usart1 */
  usart1_config(115200);

  printf("exit sleep mode by usart1 rdbf interrupt \r\n");

  while(1)
  {
    ht32_led_off(LED2);
    ht32_led_off(LED3);
    printf("now enter sleep mode \r\n");

    /* save systick register configuration */
    systick_index = SysTick->CTRL;
    systick_index &= ~((uint32_t)0xFFFFFFFE);

    /* disable systick */
    SysTick->CTRL &= (uint32_t)0xFFFFFFFE;

    /* enter sleep mode */
    pwc_sleep_mode_enter(PWC_SLEEP_ENTER_WFI);

    /* restore systick register configuration */
    SysTick->CTRL |= systick_index;

    /* wake up from sleep mode */
    printf("now exit sleep mode by usart1 rdbf interrupt \r\n");

    ht32_led_on(LED2);
    delay_ms(500);
  }
}

/**
  * @}
  */

/**
  * @}
  */
