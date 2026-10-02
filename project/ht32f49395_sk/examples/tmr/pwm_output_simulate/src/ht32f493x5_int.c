/*********************************************************************************************************//**
 * @file    ht32f493x5_int.c
 * @version $Rev:: 139         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   main interrupt service routines.
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
#include "ht32f493x5_int.h"

extern __IO uint16_t ccr1_val;
extern __IO uint16_t ccr2_val;
extern __IO uint16_t ccr3_val;
extern __IO uint16_t ccr4_val;

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_TMR_pwm_output_simulate
  * @{
  */

uint16_t capture = 0;

/**
  * @brief  this function handles nmi exception.
  * @param  none
  * @retval none
  */
void NMI_Handler(void)
{
}

/**
  * @brief  this function handles hard fault exception.
  * @param  none
  * @retval none
  */
void HardFault_Handler(void)
{
  /* go to infinite loop when hard fault exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles memory manage exception.
  * @param  none
  * @retval none
  */
void MemManage_Handler(void)
{
  /* go to infinite loop when memory manage exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles bus fault exception.
  * @param  none
  * @retval none
  */
void BusFault_Handler(void)
{
  /* go to infinite loop when bus fault exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles usage fault exception.
  * @param  none
  * @retval none
  */
void UsageFault_Handler(void)
{
  /* go to infinite loop when usage fault exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles svcall exception.
  * @param  none
  * @retval none
  */
void SVC_Handler(void)
{
}

/**
  * @brief  this function handles debug monitor exception.
  * @param  none
  * @retval none
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  this function handles pendsv_handler exception.
  * @param  none
  * @retval none
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  this function handles systick handler.
  * @param  none
  * @retval none
  */
void SysTick_Handler(void)
{
}

void TMR2_GLOBAL_IRQHandler(void)
{
  if(tmr_interrupt_flag_get(TMR2, TMR_C1_FLAG) != RESET)
  {
    tmr_flag_clear(TMR2, TMR_C1_FLAG );

    /* Pin PC.06 toggling with frequency = 73.24 Hz */
    gpio_bits_write(GPIOC, GPIO_PINS_6, (confirm_state)(1 - gpio_output_data_bit_read(GPIOC, GPIO_PINS_6)));
    capture = tmr_channel_value_get(TMR2, TMR_SELECT_CHANNEL_1);
    tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_1, capture + ccr1_val);
  }
  else if(tmr_interrupt_flag_get(TMR2, TMR_C2_FLAG) != RESET)
  {
    tmr_flag_clear(TMR2, TMR_C2_FLAG);

    /* Pin PC.06 toggling with frequency = 73.24 Hz */
    gpio_bits_write(GPIOC, GPIO_PINS_7, (confirm_state)(1 - gpio_output_data_bit_read(GPIOC, GPIO_PINS_7)));
    capture = tmr_channel_value_get(TMR2, TMR_SELECT_CHANNEL_2);
    tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_2, capture + ccr2_val);
  }
  else if(tmr_interrupt_flag_get(TMR2, TMR_C3_FLAG) != RESET)
  {
    tmr_flag_clear(TMR2, TMR_C3_FLAG);

    /* Pin PC.06 toggling with frequency = 73.24 Hz */
    gpio_bits_write(GPIOC, GPIO_PINS_8, (confirm_state)(1 - gpio_output_data_bit_read(GPIOC, GPIO_PINS_8)));
    capture = tmr_channel_value_get(TMR2, TMR_SELECT_CHANNEL_3);
    tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_3, capture + ccr3_val);
  }
  else
  {
    tmr_flag_clear(TMR2, TMR_C4_FLAG);

    /* Pin PC.06 toggling with frequency = 73.24 Hz */
    gpio_bits_write(GPIOC, GPIO_PINS_9, (confirm_state)(1 - gpio_output_data_bit_read(GPIOC, GPIO_PINS_9)));
    capture = tmr_channel_value_get(TMR2, TMR_SELECT_CHANNEL_4);
    tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_4, capture + ccr4_val);
  }
}

/**
  * @}
  */

/**
  * @}
  */
