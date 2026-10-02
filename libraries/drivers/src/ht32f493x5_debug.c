/*********************************************************************************************************//**
 * @file    ht32f493x5_debug.c
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   contains all the functions for the debug firmware library
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

#include "ht32f493x5_conf.h"

/** @addtogroup HT32F493x5_periph_driver
  * @{
  */

/** @defgroup DEBUG
  * @brief DEBUG driver modules
  * @{
  */

#ifdef DEBUG_MODULE_ENABLED

/** @defgroup DEBUG_private_functions
  * @{
  */

/**
  * @brief  get debug device id
  * @param  none
  * @retval the debug device id
  */
uint32_t debug_device_id_get(void)
{
  return DEBUGMCU->pid;
}

/**
  * @brief  set periph debug mode
  * @param  periph_debug_mode
  *         this parameter can be any combination of the following values:
  *         - DEBUG_SLEEP                           - DEBUG_DEEPSLEEP
  *         - DEBUG_STANDBY                         - DEBUG_WDT_PAUSE
  *         - DEBUG_WWDT_PAUSE                      - DEBUG_TMR1_PAUSE
  *         - DEBUG_TMR3_PAUSE                      - DEBUG_I2C1_SMBUS_TIMEOUT
  *         - DEBUG_I2C2_SMBUS_TIMEOUT              - DEBUG_I2C3_SMBUS_TIMEOUT
  *         - DEBUG_TMR2_PAUSE                      - DEBUG_TMR4_PAUSE
  *         - DEBUG_CAN1_PAUSE                      - DEBUG_TMR8_PAUSE
  *         - DEBUG_TMR5_PAUSE                      - DEBUG_TMR6_PAUSE
  *         - DEBUG_TMR7_PAUSE                      - DEBUG_CAN2_PAUSE
  *         - DEBUG_TMR12_PAUSE                     - DEBUG_TMR13_PAUSE
  *         - DEBUG_TMR14_PAUSE                     - DEBUG_TMR9_PAUSE
  *         - DEBUG_TMR10_PAUSE                     - DEBUG_TMR11_PAUSE
  * @param  new_state (TRUE or FALSE)
  * @retval none
  */
void debug_periph_mode_set(uint32_t periph_debug_mode, confirm_state new_state)
{
  if(new_state != FALSE)
  {
    DEBUGMCU->ctrl |= periph_debug_mode;
  }
  else
  {
    DEBUGMCU->ctrl &= ~periph_debug_mode;
  }
}

/**
  * @}
  */

#endif

/**
  * @}
  */

/**
  * @}
  */
