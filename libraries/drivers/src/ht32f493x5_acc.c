/*********************************************************************************************************//**
 * @file    ht32f493x5_acc.c
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   contains all the functions for the acc firmware library
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

/** @defgroup ACC
  * @brief ACC driver modules
  * @{
  */

#ifdef ACC_MODULE_ENABLED

/** @defgroup ACC_private_functions
  * @{
  */

/**
  * @brief  enable or disable the acc calibration mode.
  * @param  acc_trim: specifies the acc calibration type.
  *         this parameter can be one of the following values:
  *         - ACC_CAL_HICKCAL
  *         - ACC_CAL_HICKTRIM
  * @param  new_state: specifies the acc calibration to be enabled or disabled.(TRUE or FALSE)
  * @retval none
  */
void acc_calibration_mode_enable(uint16_t acc_trim, confirm_state new_state)
{
  if(acc_trim == ACC_CAL_HICKCAL)
  {
    ACC->ctrl1_bit.entrim = FALSE;
  }
  else
  {
    ACC->ctrl1_bit.entrim = TRUE;
  }
  ACC->ctrl1_bit.calon = new_state;
}

/**
  * @brief  store calibration step data in acc's ctrl1 register.
  * @param  step_value:  value to be stored in the acc's ctrl1 register
  * @retval none
  */
void acc_step_set(uint8_t step_value)
{
  ACC->ctrl1_bit.step = step_value;
}

/**
  * @brief  enable or disable the specified acc interrupts.
  * @param  acc_int: specifies the acc interrupt sources to be enabled or disabled.
  *         this parameter can be one of the following values:
  *         - ACC_CALRDYIEN_INT
  *         - ACC_EIEN_INT
  * @param  new_state (TRUE or FALSE)
  * @retval none
  */
void acc_interrupt_enable(uint16_t acc_int, confirm_state new_state)
{
  if(acc_int == ACC_CALRDYIEN_INT)
  {
    ACC->ctrl1_bit.calrdyien = new_state;
  }
  else
  {
    ACC->ctrl1_bit.eien = new_state;
  }
}

/**
  * @brief  return the current acc hicktrim value.
  * @param  none
  * @retval 8-bit hicktrim value.
  */
uint8_t acc_hicktrim_get(void)
{
  return ((uint8_t)(ACC->ctrl2_bit.hicktrim));
}

/**
  * @brief  return the current acc hickcal value.
  * @param  none
  * @retval 8-bit hicktrim value.
  */
uint8_t acc_hickcal_get(void)
{
  return ((uint8_t)(ACC->ctrl2_bit.hickcal));
}

/**
  * @brief  wtire the value to  acc c1 register.
  * @param  acc_c1_value
  * @retval none.
  */
void acc_write_c1(uint16_t acc_c1_value)
{
  ACC->c1 = acc_c1_value;
}

/**
  * @brief  wtire the value to  acc c2 register.
  * @param  acc_c2_value
  * @retval none.
  */
void acc_write_c2(uint16_t acc_c2_value)
{
  ACC->c2 = acc_c2_value;
}

/**
  * @brief  wtire the value to  acc c3 register.
  * @param  acc_c3_value
  * @retval none.
  */
void acc_write_c3(uint16_t acc_c3_value)
{
  ACC->c3 = acc_c3_value;
}

/**
  * @brief  return the current acc c1 value.
  * @param  none
  * @retval 16-bit c1 value.
  */
uint16_t acc_read_c1(void)
{
  return ((uint16_t)(ACC->c1));
}

/**
  * @brief  return the current acc c2 value.
  * @param  none
  * @retval 16-bit c2 value.
  */
uint16_t acc_read_c2(void)
{
  return ((uint16_t)(ACC->c2));
}

/**
  * @brief  return the current acc c3 value.
  * @param  none
  * @retval 16-bit c3 value.
  */
uint16_t acc_read_c3(void)
{
  return ((uint16_t)(ACC->c3));
}

/**
  * @brief  check whether the specified acc flag is set or not.
  * @param  acc_flag: specifies the flag to check.
  *         this parameter can be one of the following values:
  *         - ACC_RSLOST_FLAG
  *         - ACC_CALRDY_FLAG
  * @retval  flag_status (SET or RESET)
  */
flag_status acc_flag_get(uint16_t acc_flag)
{
  if(acc_flag == ACC_CALRDY_FLAG)
    return (flag_status)(ACC->sts_bit.calrdy);
  else
    return (flag_status)(ACC->sts_bit.rslost);
}

/**
  * @brief  check whether the specified acc interrupt flag is set or not.
  * @param  acc_flag: specifies the flag to check.
  *         this parameter can be one of the following values:
  *         - ACC_RSLOST_FLAG
  *         - ACC_CALRDY_FLAG
  * @retval  flag_status (SET or RESET)
  */
flag_status acc_interrupt_flag_get(uint16_t acc_flag)
{
  if(acc_flag == ACC_CALRDY_FLAG)
    return (flag_status)(ACC->sts_bit.calrdy && ACC->ctrl1_bit.calrdyien);
  else
    return (flag_status)(ACC->sts_bit.rslost && ACC->ctrl1_bit.eien);
}

/**
  * @brief  clear the specified acc flag is set or not.
  * @param  acc_flag: specifies the flag to check.
  *         this parameter can be any combination of the following values:
  *         - ACC_RSLOST_FLAG
  *         - ACC_CALRDY_FLAG
  * @retval  none
  */
void acc_flag_clear(uint16_t acc_flag)
{
  ACC->sts = ~acc_flag;
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
