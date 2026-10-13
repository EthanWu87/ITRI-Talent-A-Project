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

/** @addtogroup HT32F493x5_TMR_encoder_tmr2 TMR_encoder_tmr2
  * @{
  */

gpio_init_type gpio_init_struct = {0};
crm_clocks_freq_type crm_clocks_freq_struct = {0};

void delay(uint32_t time);
uint32_t counter;

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

  /* enable tmr2/gpioa clock */
  crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  /* timer1 output pin Configuration */
  gpio_init_struct.gpio_pins = GPIO_PINS_0 | GPIO_PINS_1;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_init_struct.gpio_pins = GPIO_PINS_2 | GPIO_PINS_3;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOA, &gpio_init_struct);

  /* tmr2 encoder mode configuration
     tmr2 ti1pf1 ,ti2fp2 as encoder input pin, tmr2 counter
     changed each signal edge. */

  /* enable tmr2 32bit function */
  tmr_32_bit_function_enable(TMR2, TRUE);

  tmr_base_init(TMR2, 0xFFFFFFFF, 0);
  tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);

  /* config encoder mode */
  tmr_encoder_mode_config(TMR2, TMR_ENCODER_MODE_C, TMR_INPUT_RISING_EDGE, TMR_INPUT_RISING_EDGE);

  /* enable tmr2 */
  tmr_counter_enable(TMR2, TRUE);

  while(1)
  {
    /* generate encoder signal */
    gpio_bits_set(GPIOA, GPIO_PINS_2);
    delay(150);
    gpio_bits_set(GPIOA, GPIO_PINS_3);
    delay(150);
    gpio_bits_reset(GPIOA, GPIO_PINS_2);
    delay(150);
    gpio_bits_reset(GPIOA, GPIO_PINS_3);
    delay(150);

    /* get current counter value */
    counter = tmr_counter_value_get(TMR2);
  }
}

/**
  * @brief  delay function
  * @param  time: number of time to delay
  * @retval none
  */
void delay(uint32_t time)
{
  volatile uint32_t i;

  for(i = 0; i < time; i++);
}

/**
  * @}
  */

/**
  * @}
  */
