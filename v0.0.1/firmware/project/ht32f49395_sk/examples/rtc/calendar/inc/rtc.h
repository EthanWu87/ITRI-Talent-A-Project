/*********************************************************************************************************//**
 * @file    rtc.h
 * @version $Rev:: 148         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   this file provides template for calendar api.
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

#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ht32f493x5.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_RTC_calendar
  * @{
  */

/** @defgroup RTC_calendar_type
  * @{
  */

typedef struct
{
  __IO uint16_t year;       /*!< specifies the years of calendar.   */
  __IO uint8_t  month;      /*!< specifies the months of calendar.  */
  __IO uint8_t  date;       /*!< specifies the date of calendar.    */
  __IO uint8_t  hour;       /*!< specifies the hours of calendar.   */
  __IO uint8_t  min;        /*!< specifies the minutes of calendar. */
  __IO uint8_t  sec;        /*!< specifies the second of calendar.  */
  __IO uint8_t  week;       /*!< specifies the weeks of calendar.   */
} calendar_type;

/**
  * @}
  */

extern calendar_type calendar;

/** @defgroup RTC_calendar_exported_functions
  * @{
  */

uint8_t rtc_init            (calendar_type *calendar);
uint8_t rtc_time_set        (calendar_type *calendar);
uint8_t rtc_alarm_clock_set (calendar_type *calendar);
void    rtc_time_get        (void);

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
