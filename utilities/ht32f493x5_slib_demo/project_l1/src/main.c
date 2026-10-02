/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 132         $
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
#include <stdio.h>
#include "math_helper.h"
#include "fir_filter.h"

extern float32_t testInput_f32_1kHz_15kHz[];
extern float32_t refOutput[];

/** @addtogroup UTILITIES_examples
  * @{
  */

/** @addtogroup SLIB_project_l1
  * @{
  */

#define SNR_THRESHOLD_F32                140.0f
#define TEST_LENGTH_SAMPLES              320

static float32_t testOutput[TEST_LENGTH_SAMPLES];

void config_flash_interrupt(void);

/**
  * @brief  config flash error interrupt.
  * @param  none
  * @retval flash status
  */
void config_flash_interrupt(void)
{
  flash_unlock();
  flash_interrupt_enable(FLASH_BANK1_ERR_INT, TRUE);
  flash_lock();
  /* config nvic priority group */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  nvic_irq_enable(FLASH_IRQn, 0, 0);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  arm_status status;
  float32_t  *inputf32, *outputf32, snr;

  system_clock_config();
  ht32_board_init();

  /* initialize input and output buffer pointers */
  inputf32 = &testInput_f32_1kHz_15kHz[0];
  outputf32 = &testOutput[0];

 /* configure flash to generate an interrupt when a write protect error occur */
  config_flash_interrupt();

  /* wait for key button to be pushed, then turn off led3 */
  while(ht32_button_press() == NO_BUTTON)
  {
    ht32_led_toggle(LED3);
    delay_ms(100);
  }
  ht32_led_off(LED3);

  /* call the fir low pass filter function */
  FIR_lowpass_filter(inputf32, outputf32, TEST_LENGTH_SAMPLES);

  /* compare the generated output against the reference output computed in matlab. */
  snr = arm_snr_f32(&refOutput[0], &testOutput[0], TEST_LENGTH_SAMPLES);
  if(snr < SNR_THRESHOLD_F32)
  {
    status = ARM_MATH_TEST_FAILURE;
  }
  else
  {
    status = ARM_MATH_SUCCESS;
  }

  /* infinite loop */
  while(1)
  {
    if(status == ARM_MATH_TEST_FAILURE)
    {
      /* toggle red led2 if fir function failed */
      ht32_led_toggle(LED2);
      delay_ms(500);
    }
    else
    {
      /* toggle green led4 if fir function success */
      ht32_led_toggle(LED4);
      delay_ms(500);
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */



