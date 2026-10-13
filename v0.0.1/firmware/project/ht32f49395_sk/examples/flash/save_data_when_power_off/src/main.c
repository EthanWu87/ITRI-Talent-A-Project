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
#include "flash.h"
#include <stdio.h>
#include <string.h>


/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_FLASH_save_data_when_power_off FLASH_save_data_when_power_off
  * @{
  */

uint32_t main_index1 = 0;
uint32_t main_index2 = 0;
uint32_t pvd_write_address = 0;
extern uint32_t save_data[8];

/**
  * @brief  power voltage monitor exint configuration.
  * @param  none
  * @retval none
  */
void pvm_exint_config(void)
{
  exint_init_type exint_init_struct;
  
  /* config the exint line of the power voltage monitor */
  exint_init_struct.line_select = EXINT_LINE_16;
  exint_init_struct.line_enable = TRUE;
  exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
  exint_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
  exint_init(&exint_init_struct);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  system_clock_config();
  ht32_board_init();

  /* turn on all led,means that code is running */
  ht32_led_on(LED2);
  ht32_led_on(LED3);
  ht32_led_on(LED4);
  delay_ms(500);

  /* enable pwc clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

  /* set the threshold voltage to 2.9v */
  pwc_pvm_level_select(PWC_PVM_VOLTAGE_2V9);

  /* enable power voltage monitor */
  pwc_power_voltage_monitor_enable(TRUE);

  /* config the exint line of the power voltage monitor */
  pvm_exint_config();

  /* enable power voltage monitor interrupt */
  nvic_irq_enable(PVM_IRQn, 0, 0);

  if(FLASH_SIZE < 256)
  {
    pvd_write_address = 0x08000000 + FLASH_SIZE * 1024 -1024;
  }
  else
  {
    pvd_write_address = 0x08000000 + FLASH_SIZE * 1024 -2048;
  }

  /* the saved data check */
  if(FLASH_SIZE < 256)
  {
    main_index1 = *(volatile uint32_t *)(0x08000000 + FLASH_SIZE * 1024 -1024);
    main_index2 = *(volatile uint32_t *)(0x08000000 + FLASH_SIZE * 1024 -1024 + 28);
  }
  else
  {
    main_index1 = *(volatile uint32_t *)(0x08000000 + FLASH_SIZE * 1024 -2048);
    main_index2 = *(volatile uint32_t *)(0x08000000 + FLASH_SIZE * 1024 -2048 + 28);
  }
  if((main_index1 == save_data[0]) && (main_index2 == save_data[7]))
  {
    /* led2 on and led3 off,means that the data is successfully saved */
    ht32_led_on(LED2);
    ht32_led_off(LED3);
  }
  else
  {
    /* led2 off and led3 on,means that their was no data being saved or data saved fail */
    ht32_led_off(LED2);
    ht32_led_on(LED3);
  }

  /* erase flash page in advance */
  while(ht32_button_press() != USER_BUTTON)
  {
  }

  /* unlock the flash controller */
  flash_unlock();
  flash_sector_erase(FLASH_BASE + FLASH_SIZE * 1024 - 2048);

  /* lock the flash controller */
  flash_lock();

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
