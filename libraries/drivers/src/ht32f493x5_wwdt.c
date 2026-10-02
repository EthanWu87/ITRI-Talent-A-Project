/*********************************************************************************************************//**
 * @file    ht32f493x5_wwdt.c
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   contains all the functions for the wwdt firmware library
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

/** @defgroup WWDT
  * @brief WWDT driver modules
  * @{
  */

#ifdef WWDT_MODULE_ENABLED

/** @defgroup WWDT_private_functions
  * @{
  */

/**
  * @brief  wwdt reset by crm reset register
  * @retval none
  */
void wwdt_reset(void)
{
  crm_periph_reset(CRM_WWDT_PERIPH_RESET, TRUE);
  crm_periph_reset(CRM_WWDT_PERIPH_RESET, FALSE);
}

/**
  * @brief  wwdt division set
  * @param  division
  *         this parameter can be one of the following values:
  *         - WWDT_PCLK1_DIV_4096        (wwdt counter clock = (pclk1/4096)/1)
  *         - WWDT_PCLK1_DIV_8192        (wwdt counter clock = (pclk1/4096)/2)
  *         - WWDT_PCLK1_DIV_16384       (wwdt counter clock = (pclk1/4096)/4)
  *         - WWDT_PCLK1_DIV_32768       (wwdt counter clock = (pclk1/4096)/8)
  * @retval none
  */
void wwdt_divider_set(wwdt_division_type division)
{
  WWDT->cfg_bit.div = division;
}

/**
  * @brief  wwdt reload counter interrupt flag clear
  * @param  none
  * @retval none
  */
void wwdt_flag_clear(void)
{
  WWDT->sts = 0;
}

/**
  * @brief  wwdt enable and the counter value load
  * @param  wwdt_cnt (0x40~0x7f)
  * @retval none
  */
void wwdt_enable(uint8_t wwdt_cnt)
{
  WWDT->ctrl = wwdt_cnt | WWDT_EN_BIT;
}

/**
  * @brief  wwdt reload counter interrupt enable
  * @param  none
  * @retval none
  */
void wwdt_interrupt_enable(void)
{
  WWDT->cfg_bit.rldien = TRUE;
}

/**
  * @brief  wwdt reload counter interrupt flag get
  * @param  none
  * @retval state of reload counter interrupt flag
  */
flag_status wwdt_flag_get(void)
{
  return (flag_status)WWDT->sts_bit.rldf;
}

/**
  * @brief  wwdt reload counter interrupt flag get
  * @param  none
  * @retval state of reload counter interrupt flag
  */
flag_status wwdt_interrupt_flag_get(void)
{
  return (flag_status)(WWDT->sts_bit.rldf && WWDT->cfg_bit.rldien);
}

/**
  * @brief  wwdt counter value set
  * @param  wwdt_cnt (0x40~0x7f)
  * @retval none
  */
void wwdt_counter_set(uint8_t wwdt_cnt)
{
  WWDT->ctrl_bit.cnt = wwdt_cnt;
}

/**
  * @brief  wwdt window counter value set
  * @param  window_cnt (0x40~0x7f)
  * @retval none
  */
void wwdt_window_counter_set(uint8_t window_cnt)
{
  WWDT->cfg_bit.win = window_cnt;
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
