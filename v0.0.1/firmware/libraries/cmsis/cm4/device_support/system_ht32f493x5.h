/*********************************************************************************************************//**
 * @file    system_ht32f493x5.h
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   cmsis cortex-m4 system header file.
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

#ifndef __SYSTEM_HT32F493x5_H
#define __SYSTEM_HT32F493x5_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup HT32F493x5_system
  * @{
  */

/** @defgroup HT32F493x5_system_clock_stable_definition
  * @{
  */

#define HEXT_STABLE_DELAY                (5000u)
#define PLL_STABLE_DELAY                 (500u)
#define SystemCoreClock                  system_core_clock
#define DUMMY_NOP()                      {__NOP();__NOP();__NOP();__NOP();__NOP(); \
                                          __NOP();__NOP();__NOP();__NOP();__NOP(); \
                                          __NOP();__NOP();__NOP();__NOP();__NOP(); \
                                          __NOP();__NOP();__NOP();__NOP();__NOP();}

/**
  * @}
  */

/** @defgroup HT32F493x5_system_exported_variables
  * @{
  */

extern unsigned int system_core_clock; /*!< system clock frequency (core clock) */

/**
  * @}
  */

/** @defgroup HT32F493x5_system_exported_functions
  * @{
  */

extern void SystemInit(void);
extern void system_core_clock_update(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
