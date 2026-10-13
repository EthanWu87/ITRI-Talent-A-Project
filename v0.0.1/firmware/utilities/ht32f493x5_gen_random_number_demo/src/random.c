/*********************************************************************************************************//**
 * @file    random.c
 * @version $Rev:: 3           $
 * @date    $Date:: 2023-01-17 #$
 * @brief   set of firmware functions to random function
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
#include "random.h"
#include "stdio.h"
#include <stdlib.h>

/** @addtogroup UTILITIES_examples
  * @{
  */

/** @addtogroup GEN_random_number_demo
  * @{
  */

/* define ht32 mcu uid address */
#define DEVICE_ID_ADDR1 0x1FFFF7E8

/**
  * @brief  get uid value as seed
  * @param  none
  * @retval uid_one_word
  */
int get_uid_for_seed (void)
{
  uint32_t   id[3] = {0};
  uint32_t   uid_one_word;

  /* get uid */
  id[0] = *(int*)DEVICE_ID_ADDR1;
  id[2] = *(int*)(DEVICE_ID_ADDR1+8);
  uid_one_word=id[0]&0x7fffffff+((id[2]&(~0x7fffffff))<<16);
  return uid_one_word;
}

/**
  * @brief  enable rtc as seed
  * @param  none
  * @retval none
  */
#if ENABLE_RTC_ASSEED
void rtc_init_for_seed (void)
{
  /* enable pwc and bpr clocks */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_BPR_PERIPH_CLOCK, TRUE);

  /* allow access to bpr domain */
  pwc_battery_powered_domain_access(TRUE);

  if (bpr_data_read(BPR_DATA1) != 0x5051)
  {
    /* reset bpr domain */
    bpr_reset();

    /* enable the lext osc */
    crm_clock_source_enable(CRM_CLOCK_SOURCE_LEXT, TRUE);
    /* wait lext is ready */
    while(crm_flag_get(CRM_LEXT_STABLE_FLAG) == RESET);
    /* select the rtc clock source */
    crm_rtc_clock_select(CRM_RTC_CLOCK_LEXT);

    /* enable rtc clock */
    crm_rtc_clock_enable(TRUE);

    /* wait for rtc registers update */
    rtc_wait_update_finish();

    /* wait for the register write to complete */
    rtc_wait_config_finish();

    /* set rtc divider: set rtc period to 1sec */
    rtc_divider_set(32767);

    /* wait for the register write to complete */
    rtc_wait_config_finish();

    /* writes data to bpr register */
    bpr_data_write(BPR_DATA1, 0x5051);
  }
  else
  {
    /* wait for rtc registers update */
    rtc_wait_update_finish();

    /* wait for the register write to complete */
    rtc_wait_config_finish();
  }
}

#endif

/**
  * @brief  random number test
  * @param  none
  * @retval none
  */
void randnum_test( void)
{
#if ENABLE_RTC_ASSEED
  rtc_init_for_seed();

  /* set uid and rtc as seed for random */
  srand(rtc_counter_get()+get_uid_for_seed());
#else
  /* set only  uid as seed for random */
  srand(get_uid_for_seed());
#endif

  while(1)
  {
    delay_ms(500);
    printf("%d\r\n",rand());
  }
}

/**
  * @}
  */

/**
  * @}
  */
