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

/** @addtogroup HT32F493x5_TMR_hall_xor_tmr2 TMR_hall_xor_tmr2
  * @{
  */

gpio_init_type  gpio_init_struct = {0};
crm_clocks_freq_type crm_clocks_freq_struct = {0};
tmr_output_config_type tmr_output_struct;
tmr_input_config_type tmr_input_config_struct;

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  ht32_board_init();

  /* get system clock */
  crm_clocks_freq_get(&crm_clocks_freq_struct);

  /* turn led2/led3/led4 on */
  ht32_led_on(LED2);
  ht32_led_on(LED3);
  ht32_led_on(LED4);

  /* enable tmr2/tmr3/gpioa/gpiob clock */
  crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  /* tmr2 channel1 ,channel2 and channel3 configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_0 | GPIO_PINS_1 | GPIO_PINS_2;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOA, &gpio_init_struct);

  /* hall signal (pa3 ,pa6 ,pa7) and trigger flag signal(pa8) configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_3 | GPIO_PINS_6 | GPIO_PINS_7 | GPIO_PINS_8;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOA, &gpio_init_struct);

  /* tmr2 xor mode configuration
  tmr2 channel1 ,channel2 and channel3 as hall signal input channel, trigger
  generated each signal edge. */

  /* tmr2 and tmr3 counter mode configuration */
  tmr_32_bit_function_enable(TMR2, TRUE);

  tmr_base_init(TMR2, 0xFFFFFFFF, 0);
  tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);

  /* config ti1 trc as input source */
  tmr_input_config_struct.input_channel_select = TMR_SELECT_CHANNEL_1;
  tmr_input_config_struct.input_mapped_select = TMR_CC_CHANNEL_MAPPED_STI;
  tmr_input_config_struct.input_polarity_select = TMR_INPUT_RISING_EDGE;
  tmr_input_channel_init(TMR2, &tmr_input_config_struct, TMR_CHANNEL_INPUT_DIV_1);

  /* xor funtion enable */
  tmr_channel1_input_select(TMR2, TMR_CHANEL1_2_3_CONNECTED_C1IRAW_XOR);

  /* select the tmr2 input trigger: C1INC */
  tmr_trigger_input_select(TMR2, TMR_SUB_INPUT_SEL_C1INC);

  /* select the slave mode: reset mode */
  tmr_sub_mode_select(TMR2, TMR_SUB_RESET_MODE);

  tmr_interrupt_enable(TMR2, TMR_TRIGGER_INT, TRUE);

  /* tmr2 trigger interrupt nvic init */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(TMR2_GLOBAL_IRQn, 0, 0);

  /* enable tmr2 */
  tmr_counter_enable(TMR2, TRUE);

  while(1)
  {
    /* generate hall signal */
    gpio_bits_set(GPIOA, GPIO_PINS_3);
    delay_us(10);
    gpio_bits_set(GPIOA, GPIO_PINS_6);
    delay_us(10);
    gpio_bits_set(GPIOA, GPIO_PINS_7);
    delay_us(10);
    gpio_bits_reset(GPIOA, GPIO_PINS_3);
    delay_us(10);
    gpio_bits_reset(GPIOA, GPIO_PINS_6);
    delay_us(10);
    gpio_bits_reset(GPIOA, GPIO_PINS_7);
    delay_us(10);
  }
}

/**
  * @brief  this function handles TMR2 trigger exception.
  * @param  none
  * @retval none
  */
void TMR2_GLOBAL_IRQHandler(void)
{
  if(tmr_interrupt_flag_get(TMR2, TMR_TRIGGER_FLAG) != RESET)
  {
    GPIOA->odt ^= GPIO_PINS_8;
    tmr_flag_clear(TMR2, TMR_TRIGGER_FLAG);
  }
}

/**
  * @}
  */

/**
  * @}
  */
