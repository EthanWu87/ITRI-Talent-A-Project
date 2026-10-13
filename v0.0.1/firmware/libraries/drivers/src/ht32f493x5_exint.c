/*********************************************************************************************************//**
 * @file    ht32f493x5_exint.c
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   contains all the functions for the exint firmware library
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

/** @defgroup EXINT
  * @brief EXINT driver modules
  * @{
  */

#ifdef EXINT_MODULE_ENABLED

/** @defgroup EXINT_private_functions
  * @{
  */

/**
  * @brief  exint reset
  * @param  none
  * @retval none
  */
void exint_reset(void)
{
  EXINT->inten = 0x00000000;
  EXINT->polcfg1 = 0x00000000;
  EXINT->polcfg2 = 0x00000000;
  EXINT->evten = 0x00000000;
  EXINT->intsts = 0x000FFFFF;
}

/**
  * @brief  exint default para init
  * @param  exint_struct
  *         - to the structure of exint_init_type
  * @retval none
  */
void exint_default_para_init(exint_init_type *exint_struct)
{
  exint_struct->line_enable = FALSE;
  exint_struct->line_select = EXINT_LINE_NONE;
  exint_struct->line_polarity = EXINT_TRIGGER_FALLING_EDGE;
  exint_struct->line_mode = EXINT_LINE_EVENT;
}

/**
  * @brief  exint init
  * @param  exint_struct
  *         - to the structure of exint_init_type
  * @retval none
  */
void exint_init(exint_init_type *exint_struct)
{
  uint32_t line_index = 0;
  line_index = exint_struct->line_select;

  EXINT->inten &= ~line_index;
  EXINT->evten &= ~line_index;

  if(exint_struct->line_enable != FALSE)
  {
    if(exint_struct->line_mode == EXINT_LINE_INTERRUPT)
    {
      EXINT->inten |= line_index;
    }
    else
    {
      EXINT->evten |= line_index;
    }

    EXINT->polcfg1 &= ~line_index;
    EXINT->polcfg2 &= ~line_index;
    if(exint_struct->line_polarity == EXINT_TRIGGER_RISING_EDGE)
    {
      EXINT->polcfg1 |= line_index;
    }
    else if(exint_struct->line_polarity == EXINT_TRIGGER_FALLING_EDGE)
    {
      EXINT->polcfg2 |= line_index;
    }
    else
    {
      EXINT->polcfg1 |= line_index;
      EXINT->polcfg2 |= line_index;
    }
  }
}

/**
  * @brief  clear exint flag
  * @param  exint_line
  *         this parameter can be any combination of the following values:
  *         - EXINT_LINE_0
  *         - EXINT_LINE_1
  *         ...
  *         - EXINT_LINE_18
  *         - EXINT_LINE_19
  * @retval none
  */
void exint_flag_clear(uint32_t exint_line)
{
  if((EXINT->swtrg & exint_line) == exint_line)
  {
    EXINT->intsts = exint_line;
    EXINT->intsts = exint_line;
  }
  else
  {
    EXINT->intsts = exint_line;
  }
}

/**
  * @brief  get exint flag
  * @param  exint_line
  *         this parameter can be one of the following values:
  *         - EXINT_LINE_0
  *         - EXINT_LINE_1
  *         ...
  *         - EXINT_LINE_18
  *         - EXINT_LINE_19
  * @retval the new state of exint flag(SET or RESET).
  */
flag_status exint_flag_get(uint32_t exint_line)
{
  flag_status status = RESET;
  uint32_t exint_flag =0;
  exint_flag = EXINT->intsts & exint_line;
  if((exint_flag != (uint16_t)RESET))
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
  * @brief  get exint interrupt flag
  * @param  exint_line
  *         this parameter can be one of the following values:
  *         - EXINT_LINE_0
  *         - EXINT_LINE_1
  *         ...
  *         - EXINT_LINE_18
  *         - EXINT_LINE_19
  * @retval the new state of exint flag(SET or RESET).
  */
flag_status exint_interrupt_flag_get(uint32_t exint_line)
{
  flag_status status = RESET;
  uint32_t exint_flag = 0;
  exint_flag = EXINT->intsts & exint_line;
  exint_flag = exint_flag & EXINT->inten;

  if((exint_flag != (uint16_t)RESET))
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
  * @brief  generate exint software interrupt event
  * @param  exint_line
  *         this parameter can be one of the following values:
  *         - EXINT_LINE_0
  *         - EXINT_LINE_1
  *         ...
  *         - EXINT_LINE_18
  *         - EXINT_LINE_19
  * @retval none
  */
void exint_software_interrupt_event_generate(uint32_t exint_line)
{
  EXINT->swtrg |= exint_line;
}

/**
  * @brief  enable or disable exint interrupt
  * @param  exint_line
  *         this parameter can be any combination of the following values:
  *         - EXINT_LINE_0
  *         - EXINT_LINE_1
  *         ...
  *         - EXINT_LINE_18
  *         - EXINT_LINE_19
  * @param  new_state: new state of exint interrupt.
  *         this parameter can be: TRUE or FALSE.
  * @retval none
  */
void exint_interrupt_enable(uint32_t exint_line, confirm_state new_state)
{
  if(new_state == TRUE)
  {
    EXINT->inten |= exint_line;
  }
  else
  {
    EXINT->inten &= ~exint_line;
  }
}

/**
  * @brief  enable or disable exint event
  * @param  exint_line
  *         this parameter can be any combination of the following values:
  *         - EXINT_LINE_0
  *         - EXINT_LINE_1
  *         ...
  *         - EXINT_LINE_18
  *         - EXINT_LINE_19
  * @param  new_state: new state of exint event.
  *         this parameter can be: TRUE or FALSE.
  * @retval none
  */
void exint_event_enable(uint32_t exint_line, confirm_state new_state)
{
  if(new_state == TRUE)
  {
    EXINT->evten |= exint_line;
  }
  else
  {
    EXINT->evten &= ~exint_line;
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
