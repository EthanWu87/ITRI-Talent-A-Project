/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 137         $
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

/** @addtogroup HT32F493x5_WWDT_reset WWDT_reset
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

  if(crm_flag_get(CRM_WWDT_RESET_FLAG) != RESET)
  {
    /* reset from wwdt */
    crm_flag_clear(CRM_WWDT_RESET_FLAG);

    ht32_led_on(LED4);
  }
  else
  {
    /* reset from other mode */
    ht32_led_off(LED4);
  }

  /* enable the wwdt clock */
  crm_periph_clock_enable(CRM_WWDT_PERIPH_CLOCK, TRUE);

  /* set the wwdt divider value */
  wwdt_divider_set(WWDT_PCLK1_DIV_32768);

  /* set the wwdt window counter value */
  wwdt_window_counter_set(0x6F);

  /* enable wwdt

   window_value:   (0x7F - 0x6F)
   timeout_value:  (0x7F - 0x40) + 1

   timeout = timeout_value * (divider / pclk1_freq )    (s)
   window  = window_value  * (divider / pclk1_freq )    (s)

   pclk1_freq   = 120 MHz
   divider      = 32768
   reload_value = 0x40 = 64
   window_value = 0x10 = 16

   timeout = 64 * (32768 / 120000000 ) = 0.0174s = 17.4ms
   window  = 16 * (32768 / 120000000 ) = 0.0044s = 4.4ms

  */
  wwdt_enable(0x7F);

  while(1)
  {
    ht32_led_toggle(LED3);

    /* the reload time must within the window (6ms > 4.4ms) */
    delay_ms(6);

    /* update the wwdt window counter */
    wwdt_counter_set(0x7F);

    if(ht32_button_press() == USER_BUTTON)
    {
      while(1);
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */
