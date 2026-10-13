/*********************************************************************************************************//**
 * @file    ht32f493x5_wdt.c
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   contains all the functions for the wdt firmware library
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

/** @defgroup WDT
  * @brief WDT driver modules
  * @{
  */

#ifdef WDT_MODULE_ENABLED

/** @defgroup WDT_private_functions
  * @{
  */

/**
  * @brief  wdt enable ,the reload value will be sent to the counter
  * @param  none
  * @retval none
  */
void wdt_enable(void)
{
  WDT->cmd = WDT_CMD_ENABLE;
}

/**
  * @brief  reload wdt counter
  * @param  none
  * @retval none
  */
void wdt_counter_reload(void)
{
  WDT->cmd = WDT_CMD_RELOAD;
}

/**
  * @brief  set wdt counter reload value
  * @param  reload_value (0x0000~0x0FFF)
  * @retval none
  */
void wdt_reload_value_set(uint16_t reload_value)
{
  WDT->rld = reload_value;
}

/**
  * @brief  set wdt division divider
  * @param  division
  *         this parameter can be one of the following values:
  *         - WDT_CLK_DIV_4
  *         - WDT_CLK_DIV_8
  *         - WDT_CLK_DIV_16
  *         - WDT_CLK_DIV_32
  *         - WDT_CLK_DIV_64
  *         - WDT_CLK_DIV_128
  *         - WDT_CLK_DIV_256
  * @retval none
  */
void wdt_divider_set(wdt_division_type division)
{
  WDT->div_bit.div = division;
}

/**
  * @brief  enable or disable wdt cmd register write
  * @param  new_state (TRUE or FALSE)
  * @retval none
  */
void wdt_register_write_enable( confirm_state new_state)
{
  if(new_state == FALSE)
  {
    WDT->cmd = WDT_CMD_LOCK;
  }
  else
  {
    WDT->cmd = WDT_CMD_UNLOCK;
  }
}

/**
  * @brief  get wdt flag
  * @param  wdt_flag
  *         this parameter can be one of the following values:
  *         - WDT_DIVF_UPDATE_FLAG: division value update complete flag.
  *         - WDT_RLDF_UPDATE_FLAG: reload value update complete flag.
  * @retval state of wdt flag
  */
flag_status wdt_flag_get(uint16_t wdt_flag)
{
  flag_status status = RESET;

  if ((WDT->sts & wdt_flag) != (uint16_t)RESET)
  {
    status = SET;
  }
  else
  {
    status = RESET;
  }

  return status;
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
