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

/** @addtogroup HT32F493x5_PWC_standby_rtc PWC_standby_rtc
  * @{
  */


/**
  * @brief  rtc configuration.
  * @param  none
  * @retval none
  */
void rtc_config(void)
{
  /* enable the battery-powered domain write operations */
  pwc_battery_powered_domain_access(TRUE);

  /* reset battery-powered domain register */
  bpr_reset();

  /* enable the lick */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_LICK, TRUE);

  /* wait lick is ready */
  while(crm_flag_get(CRM_LICK_STABLE_FLAG) == RESET);

  /* select the rtc clock source */
  crm_rtc_clock_select(CRM_RTC_CLOCK_LICK);

  /* enable rtc clock */
  crm_rtc_clock_enable(TRUE);

  /* wait for rtc registers update */
  rtc_wait_update_finish();

  /* set rtc divider: set rtc period to 1sec */
  rtc_divider_set(32767);

  /* wait for the register write to complete */
  rtc_wait_config_finish();
}

/**
  * @brief  rtc alarm set.
  * @param  none
  * @retval none
  */
void rtc_alarm_config(uint8_t alarm_time)
{
  /* clear second flag */
  rtc_flag_clear(RTC_TS_FLAG);

  /* wait for the second flag to be set */
  while(rtc_flag_get(RTC_TS_FLAG) == RESET);

  /* config the wakeup time */
  rtc_alarm_set(rtc_counter_get() + alarm_time);

  /* wait for the register write to complete */
  rtc_wait_config_finish();
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  __IO uint32_t index = 0;

  /* congfig the system clock */
  system_clock_config();

  /* init ht32 starter kit */
  ht32_board_init();

  /* config priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  /* turn on the led light */
  ht32_led_off(LED2);
  ht32_led_off(LED3);
  ht32_led_off(LED4);

  /* enable pwc and bpr clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_BPR_PERIPH_CLOCK, TRUE);

  if(pwc_flag_get(PWC_STANDBY_FLAG) != RESET)
  {
    /* wakeup from standby */
    pwc_flag_clear(PWC_STANDBY_FLAG);
    ht32_led_on(LED2);
  }

  if(pwc_flag_get(PWC_WAKEUP_FLAG) != RESET)
  {
    /* wakeup event occurs */
    pwc_flag_clear(PWC_WAKEUP_FLAG);
    ht32_led_on(LED3);
  }

  /* config rtc */
  rtc_config();

  ht32_led_on(LED4);

  delay_ms(1000);

  /* set the wakeup time to 10 seconds */
  rtc_alarm_config(10);

  /* enter standby mode */
  pwc_standby_mode_enter();

  while(1)
  {
  }
}

/**
  * @}
  */

/**
  * @}
  */
