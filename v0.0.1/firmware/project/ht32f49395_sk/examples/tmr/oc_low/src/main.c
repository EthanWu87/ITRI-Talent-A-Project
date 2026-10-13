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

/** @addtogroup HT32F493x5_TMR_oc_low TMR_oc_low
  * @{
  */

tmr_output_config_type tmr_oc_init_structure;
uint16_t ccr1_val = 1000;
uint16_t ccr2_val = 500;
uint16_t ccr3_val = 250;
uint16_t ccr4_val = 125;
uint16_t prescaler_value = 0;

void gpio_configuration(void);

/**
  * @brief  configure the gpioc pins.
  * @param  none
  * @retval none
  */
void gpio_configuration(void)
{
  gpio_init_type gpio_init_struct;

  /* gpioc configuration: pin6, 7, 8 and 9 as output push-pull */
  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_pins = GPIO_PINS_6 | GPIO_PINS_7 | GPIO_PINS_8 | GPIO_PINS_9;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

  gpio_init(GPIOC, &gpio_init_struct);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* config nvic priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  system_clock_config();

  ht32_board_init();

  crm_apb1_div_set(CRM_APB1_DIV_8);
  crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

  nvic_irq_enable(TMR2_GLOBAL_IRQn, 0, 0);

  gpio_configuration();

  /* compute the prescaler value */
  prescaler_value = (uint16_t)(system_core_clock / 4000) - 1;

  /* tmr2 time base configuration */
  tmr_base_init(TMR2, 65535, prescaler_value);
  tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);
  tmr_clock_source_div_set(TMR2, TMR_CLOCK_DIV1);

  tmr_output_default_para_init(&tmr_oc_init_structure);
  tmr_oc_init_structure.oc_mode = TMR_OUTPUT_CONTROL_LOW;
  tmr_oc_init_structure.oc_idle_state = FALSE;
  tmr_oc_init_structure.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
  tmr_oc_init_structure.oc_output_state = TRUE;
  tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_1, &tmr_oc_init_structure);
  tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_1, ccr1_val);

  tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_2, &tmr_oc_init_structure);
  tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_2, ccr2_val);

  tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_3, &tmr_oc_init_structure);
  tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_3, ccr3_val);

  tmr_output_channel_config(TMR2, TMR_SELECT_CHANNEL_4, &tmr_oc_init_structure);
  tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_4, ccr4_val);

  tmr_period_buffer_enable(TMR2, TRUE);

  /* tmr2 int enable */
  tmr_interrupt_enable(TMR2, TMR_C1_INT | TMR_C2_INT | TMR_C3_INT | TMR_C4_INT, TRUE);

  /* set pc.06, pc.07, pc.08 and pc.09 pins */
  gpio_bits_set(GPIOC, GPIO_PINS_6 | GPIO_PINS_7 | GPIO_PINS_8 | GPIO_PINS_9);

  /* tmr2 enable counter */
  tmr_counter_enable(TMR2, TRUE);

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
