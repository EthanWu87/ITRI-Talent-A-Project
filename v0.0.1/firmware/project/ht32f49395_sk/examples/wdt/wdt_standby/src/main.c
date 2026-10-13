/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 136         $
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

/** @addtogroup HT32F493x5_WDT_standby WDT_standby
  * @{
  */


/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  ht32_board_init();

  /* enable the pwc clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

  if(crm_flag_get(CRM_WDT_RESET_FLAG) != RESET)
  {
    /* reset from wdt */
    crm_flag_clear(CRM_WDT_RESET_FLAG);

    ht32_led_on(LED4);
  }
  else
  {
    /* reset from other mode */
    ht32_led_off(LED4);
  }

  delay_ms(100);

  /* disable register write protection */
  wdt_register_write_enable(TRUE);

  /* set the wdt divider value */
  wdt_divider_set(WDT_CLK_DIV_4);

  /* set reload value

   timeout = reload_value * (divider / lick_freq )    (s)

   lick_freq    = 40000 Hz
   divider      = 4
   reload_value = 3000

   timeout = 3000 * (4 / 40000 ) = 0.3s = 300ms
  */
  wdt_reload_value_set(3000 - 1);

  /* reload wdt counter */
  wdt_counter_reload();
  
  /* enable wdt */
  wdt_enable();

  /* enter standby mode */
  pwc_standby_mode_enter();

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
