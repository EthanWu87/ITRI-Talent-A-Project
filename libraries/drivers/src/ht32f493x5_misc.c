/*********************************************************************************************************//**
 * @file    ht32f493x5_misc.c
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   contains all the functions for the misc firmware library
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

/* includes ------------------------------------------------------------------*/
#include "ht32f493x5_conf.h"

/** @addtogroup HT32F493x5_periph_driver
  * @{
  */

/** @defgroup MISC
  * @brief MISC driver modules
  * @{
  */

#ifdef MISC_MODULE_ENABLED

/** @defgroup MISC_private_functions
  * @{
  */

#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)

/**
  * @brief  system reset
  * @param  none
  * @retval none
  */
void nvic_system_reset(void)
{
  NVIC_SystemReset();
}

/**
  * @brief  enable nvic irq
  * @param  irqn (IRQn_Type number)
  * @param  preempt_priority: preemptive priority value (starting from 0)
  * @param  sub_priority: subpriority value (starting from 0)
  * @retval none
  */
void nvic_irq_enable(IRQn_Type irqn, uint32_t preempt_priority, uint32_t sub_priority)
{
  uint32_t temp_priority = 0;

  /* encode priority */
  temp_priority = NVIC_EncodePriority(NVIC_GetPriorityGrouping(), preempt_priority, sub_priority);
  /* set priority */
  NVIC_SetPriority(irqn, temp_priority);
  /* enable irqn */
  NVIC_EnableIRQ(irqn);
}

/**
  * @brief  disable nvic irq number
  * @param  irqn (IRQn_Type number)
  * @retval none
  */
void nvic_irq_disable(IRQn_Type irqn)
{
  NVIC_DisableIRQ(irqn);
}

/**
  * @brief  config nvic priority group
  * @param  priority_group
  *         this parameter can be one of the following values:
  *         - NVIC_PRIORITY_GROUP_0
  *         - NVIC_PRIORITY_GROUP_1
  *         - NVIC_PRIORITY_GROUP_2
  *         - NVIC_PRIORITY_GROUP_3
  *         - NVIC_PRIORITY_GROUP_4
  * @retval none
  */
void nvic_priority_group_config(nvic_priority_group_type priority_group)
{
  /* set the prigroup[10:8] bits according to nvic_prioritygroup value */
  NVIC_SetPriorityGrouping(priority_group);
}

/**
  * @brief  set the vector table location and offset.
  * @param  base
  *         this parameter can be one of the following values:
  *         - NVIC_VECTTAB_RAM
  *         - NVIC_VECTTAB_FLASH
  * @param  offset (vector table base offset field. this value must be a multiple of 0x200)
  * @retval none
  */
void nvic_vector_table_set(uint32_t base, uint32_t offset)
{
  SCB->VTOR = base | (offset & (uint32_t)0x1FFFFF80);
}

/**
  * @brief  config nvic lowpower mode
  * @param  lp_mode
  *         this parameter can be one of the following values:
  *         - NVIC_LP_SEVONPEND
  *         - NVIC_LP_SLEEPDEEP
  *         - NVIC_LP_SLEEPONEXIT
  * @param  new_state (new state of lp condition. ENABLE or DISABLE)
  * @retval none
  */
void nvic_lowpower_mode_config(nvic_lowpower_mode_type lp_mode, confirm_state new_state)
{
  if(new_state != FALSE)
  {
    SCB->SCR |= lp_mode;
  }
  else
  {
    SCB->SCR &= (uint32_t)(~(uint32_t)lp_mode);
  }
}

/**
  * @brief  config systick clock source
  * @param  source
  *         this parameter can be one of the following values:
  *         - SYSTICK_CLOCK_SOURCE_AHBCLK_DIV8
  *         - SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV
  * @retval none
  */
void systick_clock_source_config(systick_clock_source_type source)
{
  if(source == SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV)
  {
    SysTick->CTRL |= SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV;
  }
  else
  {
    SysTick->CTRL &= ~(uint32_t)SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV;
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


