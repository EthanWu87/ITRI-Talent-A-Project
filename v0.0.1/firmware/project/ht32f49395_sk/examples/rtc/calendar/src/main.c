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
#include "rtc.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_RTC_calendar RTC_calendar
  * @{
  */

char const weekday_table[7][10] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

/**
  * @brief  init alarm.
  * @param  none
  * @retval none
  */
void alarm_init(void)
{
  calendar_type alarm_struct;

  /* clear alarm flag */
  rtc_flag_clear(RTC_TA_FLAG);

  /* wait for the register write to complete */
  rtc_wait_config_finish();

  /* configure and enable rtc interrupt */
  nvic_irq_enable(RTC_IRQn, 0, 0);

  /* enable alarm interrupt */
  rtc_interrupt_enable(RTC_TA_INT, TRUE);

  /* wait for the register write to complete */
  rtc_wait_config_finish();

  /* config alarm */

  alarm_struct.year  = 2021;
  alarm_struct.month = 5;
  alarm_struct.date  = 1;
  alarm_struct.hour  = 12;
  alarm_struct.min   = 0;
  alarm_struct.sec   = 5;

  rtc_alarm_clock_set(&alarm_struct);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  calendar_type time_struct;

  /* config nvic priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  system_clock_config();

  ht32_board_init();

  uart_print_init(115200);

  /* config calendar */
  time_struct.year  = 2021;
  time_struct.month = 5;
  time_struct.date  = 1;
  time_struct.hour  = 12;
  time_struct.min   = 0;
  time_struct.sec   = 0;
  rtc_init(&time_struct);

  /* config alarm */
  alarm_init();

  printf("initial ok\r\n");

  while(1)
  {
    if(rtc_flag_get(RTC_TS_FLAG) != RESET)
    {
      ht32_led_toggle(LED3);

      /* get time */
      rtc_time_get();

      /* print time */
      printf("%d/%d/%d ", calendar.year, calendar.month, calendar.date);
      printf("%02d:%02d:%02d %s\r\n", calendar.hour, calendar.min, calendar.sec, weekday_table[calendar.week]);

      /* wait for the register write to complete */
      rtc_wait_config_finish();

      /* clear the rtc second flag */
      rtc_flag_clear(RTC_TS_FLAG);

      /* wait for the register write to complete */
      rtc_wait_config_finish();
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */
