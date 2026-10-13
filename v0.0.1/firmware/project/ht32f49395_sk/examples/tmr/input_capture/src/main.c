/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 139         $
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

/** @addtogroup HT32F493x5_TMR_input_capture TMR_input_capture
  * @{
  */

gpio_init_type  gpio_init_struct = {0};
crm_clocks_freq_type crm_clocks_freq_struct = {0};
tmr_input_config_type  tmr_input_config_struct;
void delay(uint32_t time);
void uart_init(uint32_t baudrate);
__IO uint32_t tmr3freq = 0;
__IO uint32_t sys_counter = 0;
__IO uint16_t capturenumber = 0;
__IO uint16_t ic3readvalue1 = 0, ic3readvalue2 = 0;
__IO uint32_t capture = 0;

/**
  * @brief  delay function
  * @param  time:number of time to delay
  * @retval none
  */
void delay(uint32_t time)
{
  volatile uint32_t i;

  for(i = 0; i < time; i++);
}

/**
  * @brief  this function handles tmr3 trigger exception.
  * @param  none
  * @retval none
  */
void TMR3_GLOBAL_IRQHandler(void)
{
  if(tmr_interrupt_flag_get(TMR3, TMR_C2_FLAG) != RESET)
  {
    tmr_flag_clear(TMR3, TMR_C2_FLAG);
    if(capturenumber == 0)
    {
      /* get the Input Capture value */
      ic3readvalue1 = tmr_channel_value_get(TMR3, TMR_SELECT_CHANNEL_2);
      capturenumber = 1;
    }
    else if(capturenumber == 1)
    {
      /* get the Input Capture value */
      ic3readvalue2 = tmr_channel_value_get(TMR3, TMR_SELECT_CHANNEL_2);

      /* capture computation */
      if(ic3readvalue2 > ic3readvalue1)
      {
        capture = (ic3readvalue2 - ic3readvalue1);
      }
      else
      {
        capture = ((0x10000 - ic3readvalue1) + ic3readvalue2);
      }

      /* frequency computation */
      tmr3freq = (uint32_t) crm_clocks_freq_struct.sclk_freq / capture;
      capturenumber = 0;
    }
  }
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  ht32_board_init();

  uart_print_init(115200);

  /* get system clock */
  crm_clocks_freq_get(&crm_clocks_freq_struct);

  /* turn led2/led3/led4 on */
  ht32_led_on(LED2);
  ht32_led_on(LED3);
  ht32_led_on(LED4);

  /* enable tmr3/gpioa clock */
  crm_periph_clock_enable(CRM_TMR3_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  /* timer3 input pin Configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_7;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOA, &gpio_init_struct);

  /* tmr3 configuration: input capture mode
     the external signal is connected to tmr3 ch2 pin (pa.07)
     the rising edge is used as active edge,
     the tmr3 c2dt is used to compute the frequency value */

  /* tmr3 counter mode configuration */
  tmr_base_init(TMR3, 0xFFFF, 0);
  tmr_cnt_dir_set(TMR3, TMR_COUNT_UP);

  /* configure tmr3 channel2 to get clock signal */
  tmr_input_config_struct.input_channel_select = TMR_SELECT_CHANNEL_2;
  tmr_input_config_struct.input_mapped_select = TMR_CC_CHANNEL_MAPPED_DIRECT;
  tmr_input_config_struct.input_polarity_select = TMR_INPUT_RISING_EDGE;
  tmr_input_channel_init(TMR3, &tmr_input_config_struct, TMR_CHANNEL_INPUT_DIV_1);

  tmr_interrupt_enable(TMR3, TMR_C2_INT, TRUE);

  /* tmr2 trigger interrupt nvic init */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(TMR3_GLOBAL_IRQn, 0, 0);

  /* enable tmr3 */
  tmr_counter_enable(TMR3, TRUE);

  while(1)
  {
    delay(10000);
    printf("The external signal frequece is : %d\r\n",tmr3freq);
    tmr3freq = 0;
  }
}

/**
  * @}
  */

/**
  * @}
  */
