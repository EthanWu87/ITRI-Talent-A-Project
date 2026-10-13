/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 142         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   main program
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

#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_PWC_sleep_tmr2 PWC_sleep_tmr2
  * @{
  */


/**
  * @brief  tmr2 configuration.
  * @param  none
  * @retval none
  */
void tmr2_config(void)
{
  crm_clocks_freq_type crm_clocks_freq_struct = {0};

  /* enable tmr 2 clock */
  crm_periph_clock_enable(CRM_TMR2_PERIPH_CLOCK, TRUE);

  /* get system clock */
  crm_clocks_freq_get(&crm_clocks_freq_struct);

  /* (systemclock/(systemclock/10000))/10000 = 1Hz(1s) */
  tmr_base_init(TMR2, 9999, (crm_clocks_freq_struct.sclk_freq/10000 - 1));

  /* config the counting direction */
  tmr_cnt_dir_set(TMR2, TMR_COUNT_UP);

  /* config the clock divider value */
  tmr_clock_source_div_set(TMR2, TMR_CLOCK_DIV1);

  /* enable tmr 2 interrupt */
  tmr_interrupt_enable(TMR2, TMR_OVF_INT, TRUE);

  /* config tmr 2 nvic */
  nvic_irq_enable(TMR2_GLOBAL_IRQn, 0, 0);

  /* enable tmr 2 */
  tmr_counter_enable(TMR2, TRUE);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  __IO uint32_t index = 0;
  __IO uint32_t systick_index = 0;

  /* congfig the system clock */
  system_clock_config();

  /* init ht32 starter kit */
  ht32_board_init();

  /* config priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  /* turn on the led light */
  ht32_led_on(LED2);
  ht32_led_on(LED3);
  ht32_led_on(LED4);

  /* enable pwc clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

  /* config tmr 2 */
  tmr2_config();

  while(1)
  {
    ht32_led_off(LED2);

    /* save systick register configuration */
    systick_index = SysTick->CTRL;
    systick_index &= ~((uint32_t)0xFFFFFFFE);

    /* disable systick */
    SysTick->CTRL &= (uint32_t)0xFFFFFFFE;

    /* enter sleep mode */
    pwc_sleep_mode_enter(PWC_SLEEP_ENTER_WFI);

    /* restore systick register configuration */
    SysTick->CTRL |= systick_index;

    /* wake up from sleep mode */
    ht32_led_on(LED2);
    delay_ms(500);
  }
}

/**
  * @}
  */

/**
  * @}
  */
