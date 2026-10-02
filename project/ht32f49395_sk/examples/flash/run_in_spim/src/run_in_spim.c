/*********************************************************************************************************//**
 * @file    run_in_spim.c
 * @version $Rev:: 148         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   run in spim program
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
#include "run_in_spim.h"


/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_FLASH_run_in_spim
  * @{
  */


/**
  * @brief  check the led toggle in spim
  * @param  none
  * @retval none
  */
void spim_run(void)
{
  while(1)
  {
    /* toggle led */
    ht32_led_toggle(LED2);
    delay_ms(100);
    ht32_led_toggle(LED3);
    delay_ms(100);
    ht32_led_toggle(LED4);
    delay_ms(100);
  }
}
/**
  * @}
  */

/**
  * @}
  */
