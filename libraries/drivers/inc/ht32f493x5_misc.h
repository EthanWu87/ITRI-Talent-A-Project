/*********************************************************************************************************//**
 * @file    ht32f493x5_misc.h
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   ht32f493x5 misc header file
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

/* define to prevent recursive inclusion -------------------------------------*/
#ifndef __HT32F493x5_MISC_H
#define __HT32F493x5_MISC_H

#ifdef __cplusplus
extern "C" {
#endif


/* includes ------------------------------------------------------------------*/
#include "ht32f493x5.h"

/** @addtogroup HT32F493x5_periph_driver
  * @{
  */

/** @addtogroup MISC
  * @{
  */

/** @defgroup MISC_vector_table_base_address
  * @{
  */

#define NVIC_VECTTAB_RAM                 ((uint32_t)0x20000000) /*!< nvic vector table based ram address */
#define NVIC_VECTTAB_FLASH               ((uint32_t)0x08000000) /*!< nvic vector table based flash address */

/**
  * @}
  */

/** @defgroup MISC_exported_types
  * @{
  */

/**
  * @brief nvic interrupt priority group
  */
typedef enum
{
  NVIC_PRIORITY_GROUP_0                  = ((uint32_t)0x7), /*!< 0 bits for preemption priority, 4 bits for subpriority */
  NVIC_PRIORITY_GROUP_1                  = ((uint32_t)0x6), /*!< 1 bits for preemption priority, 3 bits for subpriority */
  NVIC_PRIORITY_GROUP_2                  = ((uint32_t)0x5), /*!< 2 bits for preemption priority, 2 bits for subpriority */
  NVIC_PRIORITY_GROUP_3                  = ((uint32_t)0x4), /*!< 3 bits for preemption priority, 1 bits for subpriority */
  NVIC_PRIORITY_GROUP_4                  = ((uint32_t)0x3)  /*!< 4 bits for preemption priority, 0 bits for subpriority */
} nvic_priority_group_type;

/**
  * @brief nvic low power mode
  */
typedef enum
{
  NVIC_LP_SLEEPONEXIT                    = 0x02, /*!< enable sleep-on-exit feature */
  NVIC_LP_SLEEPDEEP                      = 0x04, /*!< enable sleep-deep output signal when entering sleep mode */
  NVIC_LP_SEVONPEND                      = 0x10  /*!< send event on pending */
} nvic_lowpower_mode_type;

/**
  * @brief systick clock source
  */
typedef enum
{
  SYSTICK_CLOCK_SOURCE_AHBCLK_DIV8       = ((uint32_t)0x00000000), /*!< systick clock source from core clock div8 */
  SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV      = ((uint32_t)0x00000004)  /*!< systick clock source from core clock */
} systick_clock_source_type;

/**
  * @}
  */

/** @defgroup MISC_exported_functions
  * @{
  */

void nvic_system_reset(void);
void nvic_irq_enable(IRQn_Type irqn, uint32_t preempt_priority, uint32_t sub_priority);
void nvic_irq_disable(IRQn_Type irqn);
void nvic_priority_group_config(nvic_priority_group_type priority_group);
void nvic_vector_table_set(uint32_t base, uint32_t offset);
void nvic_lowpower_mode_config(nvic_lowpower_mode_type lp_mode, confirm_state new_state);
void systick_clock_source_config(systick_clock_source_type source);

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
