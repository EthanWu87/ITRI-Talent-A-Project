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

/** @addtogroup HT32F493x5_TMR_7_pwm_output TMR_7_pwm_output
  * @{
  */

gpio_init_type  gpio_init_struct = {0};
tmr_output_config_type tmr_output_struct;
crm_clocks_freq_type crm_clocks_freq_struct = {0};

uint16_t timer_period = 0;
uint16_t channel1_pulse = 0, channel2_pulse = 0, channel3_pulse = 0, channel4_pulse = 0;

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

  /* enable tmr1/gpioa/gpiob clock */
  crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  /* timer1 output pin Configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_8 | GPIO_PINS_9 | GPIO_PINS_10 | GPIO_PINS_11;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_init_struct.gpio_pins = GPIO_PINS_13 | GPIO_PINS_14 | GPIO_PINS_15;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOB, &gpio_init_struct);

  gpio_init_struct.gpio_pins = GPIO_PINS_12;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOB, &gpio_init_struct);


  /* tmr1 configuration generate 7 pwm signals with 4 different duty cycles:
   prescaler = 0, tmr1 counter clock = system_core_clock

   the objective is to generate 7 pwm signal at 17.57 khz:
     - tim1_period = (system_core_clock / 17570) - 1
   the channel 1 and channel 1n duty cycle is set to 50%
   the channel 2 and channel 2n duty cycle is set to 37.5%
   the channel 3 and channel 3n duty cycle is set to 25%
   the channel 4 duty cycle is set to 12.5%
   the timer pulse is calculated as follows:
     - channelxpulse = duty_cycle * (tim1_period - 1) / 100 */

  /* compute the value to be set in arr regiter to generate signal frequency at 17.57 khz */
  timer_period = (crm_clocks_freq_struct.sclk_freq / 17570 ) - 1;

  /* compute ccr1 value to generate a duty cycle at 50% for channel 1 and 1n */
  channel1_pulse = (uint16_t)(((uint32_t) 5 * (timer_period - 1)) / 10);

  /* compute ccr2 value to generate a duty cycle at 37.5%  for channel 2 and 2n */
  channel2_pulse = (uint16_t)(((uint32_t) 375 * (timer_period - 1)) / 1000);

  /* compute ccr3 value to generate a duty cycle at 25%  for channel 3 and 3n */
  channel3_pulse = (uint16_t)(((uint32_t) 25 * (timer_period - 1)) / 100);

  /* compute ccr4 value to generate a duty cycle at 12.5%  for channel 4 */
  channel4_pulse = (uint16_t)(((uint32_t) 125 * (timer_period- 1)) / 1000);

  tmr_base_init(TMR1, timer_period, 0);
  tmr_cnt_dir_set(TMR1, TMR_COUNT_UP);

  /* channel 1, 2, 3 and 4 configuration in output mode */
  tmr_output_default_para_init(&tmr_output_struct);
  tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_B;
  tmr_output_struct.oc_output_state = TRUE;
  tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_LOW;
  tmr_output_struct.oc_idle_state = TRUE;
  tmr_output_struct.occ_output_state = TRUE;
  tmr_output_struct.occ_polarity = TMR_OUTPUT_ACTIVE_HIGH;
  tmr_output_struct.occ_idle_state = FALSE;

  /* channel 1 */
  tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, channel1_pulse);

  /* channel 2 */
  tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_2, &tmr_output_struct);
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_2, channel2_pulse);

  /* channel 3 */
  tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_3, &tmr_output_struct);
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_3, channel3_pulse);

  /* channel 4 */
  tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_4, &tmr_output_struct);
  tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_4, channel4_pulse);

  /* output enable */
  tmr_output_enable(TMR1, TRUE);

  /* enable tmr1 */
  tmr_counter_enable(TMR1, TRUE);

  while(1)
  {
  }
}

/**
  * @}
  */

/**
  * @}
  */
