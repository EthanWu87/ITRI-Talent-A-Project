/*********************************************************************************************************//**
 * @file    ht32f493x5_int.c
 * @version $Rev:: 143         $
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
#include "ht32f493x5_board.h"
#include "stdio.h"

extern uint32_t incrementvar_operationcomplete(void);
extern uint32_t getvar_operationcomplete(void);
extern void setvar_periodvalue(uint32_t Value);

extern __IO uint32_t periodvalue;
extern __IO uint32_t operationcomplete;

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_RTC_lick_calibration
  * @{
  */

uint16_t tmpCC4[2] = {0, 0};

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

/**
  * @brief  this function handles rtc global interrupt request.
  * @param  none
  * @retval none
  */
void RTC_IRQHandler(void)
{
  if(rtc_interrupt_flag_get(RTC_TS_FLAG) != RESET)
  {
    /* toggle led3 */
    ht32_led_toggle(LED3);

    /* clear interrupt pending bit */
    rtc_flag_clear(RTC_TS_FLAG);

    printf("%d\r\n", rtc_counter_get());
  }
}

/**
  * @brief  this function handles tmr5 global interrupt request.
  * @param  none
  * @retval none
  */
void TMR5_GLOBAL_IRQHandler(void)
{
  uint32_t tmp = 0;

  if(tmr_interrupt_flag_get(TMR5, TMR_C4_FLAG) == SET)
  {
    tmpCC4[incrementvar_operationcomplete()] = (uint16_t)(TMR5->c4dt);

    tmr_interrupt_flag_get(TMR5, TMR_C4_FLAG);

    if(getvar_operationcomplete() >= 2)
    {
      /* compute the period length */
      tmp = (uint16_t)(tmpCC4[1] - tmpCC4[0] + 1);
      setvar_periodvalue(tmp);

      /* disable the interrupt */
      tmr_interrupt_enable(TMR5, TMR_C4_INT, FALSE);
      tmr_counter_enable(TMR5, FALSE);
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */
