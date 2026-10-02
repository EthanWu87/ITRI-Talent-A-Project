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
#include "julia_fpu.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_CORTEX_m4_fpu CORTEX_m4_fpu
  * @{
  */

#define  SCREEN_X_SIZE                   ((uint16_t)320)
#define  SCREEN_Y_SIZE                   ((uint16_t)240)
#define  ZOOM                            ((uint16_t)100)

uint8_t buffer[SCREEN_X_SIZE * SCREEN_Y_SIZE];

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  ht32_board_init();

  while(1)
  {
    ht32_led_toggle(LED4);

    generate_julia_fpu(SCREEN_X_SIZE, SCREEN_Y_SIZE, SCREEN_X_SIZE / 2, SCREEN_Y_SIZE / 2, ZOOM, buffer);
  }
}

/**
  * @}
  */

/**
  * @}
  */
