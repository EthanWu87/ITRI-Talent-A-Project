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

/** @addtogroup HT32F493x5_GPIO_io_toggle GPIO_io_toggle
  * @{
  */

/**
  * @brief  pa.01 gpio configuration.
  * @param  none
  * @retval none
  */
void gpio_config(void)
{
  gpio_init_type gpio_init_struct;

  /* enable the gpioa clock */
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  /* set default parameter */
  gpio_default_para_init(&gpio_init_struct);

  /* configure the gpio */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_1;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  gpio_config();

  while(1)
  {
    /* set pa.01 */
    GPIOA->scr = GPIO_PINS_1;
    /* reset pa.01 */
    GPIOA->clr = GPIO_PINS_1;

    /* set pa.01 */
    GPIOA->scr = GPIO_PINS_1;
    /* reset pa.01 */
    GPIOA->clr = GPIO_PINS_1;

    /* set pa.01 */
    GPIOA->scr = GPIO_PINS_1;
    /* reset pa.01 */
    GPIOA->clr = GPIO_PINS_1;

    /* set pa.01 */
    GPIOA->scr = GPIO_PINS_1;
    /* reset pa.01 */
    GPIOA->clr = GPIO_PINS_1;

    /* set pa.01 */
    GPIOA->scr = GPIO_PINS_1;
    /* reset pa.01 */
    GPIOA->clr = GPIO_PINS_1;

    /* set pa.01 */
    GPIOA->scr = GPIO_PINS_1;
    /* reset pa.01 */
    GPIOA->clr = GPIO_PINS_1;
  }
}

/**
  * @}
  */

/**
  * @}
  */
