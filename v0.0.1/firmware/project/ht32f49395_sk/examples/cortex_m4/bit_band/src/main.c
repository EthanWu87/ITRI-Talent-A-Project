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

/** @addtogroup HT32F493x5_CORTEX_m4_bit_band CORTEX_m4_bit_band
  * @{
  */


/* bit band for sram  */
#define RAM_BASE                         0x20000000
#define RAM_BITBAND_BASE                 0x22000000

#define VARIABLES_RESET_BIT(variables_addr, bit_number)    \
        (*(uint32_t *)(RAM_BITBAND_BASE + ((variables_addr - RAM_BASE) * 32) + ((bit_number) * 4)) = 0)

#define VARIABLES_SET_BIT(variables_addr, bit_number)       \
        (*(uint32_t *)(RAM_BITBAND_BASE + ((variables_addr - RAM_BASE) * 32) + ((bit_number) * 4)) = 1)

#define VARIABLES_GET_BIT(variables_addr, bit_number)       \
        (*(uint32_t *)(RAM_BITBAND_BASE + ((variables_addr - RAM_BASE) * 32) + ((bit_number) * 4)))

/* bit band for peripheral  */
#define PERIPHERAL_BASE                  0x40000000
#define PERIPHERAL_BITBAND_BASE          0x42000000

#define PERIPHERAL_RESET_BIT(peripheral_addr, bit_number)    \
        (*(uint32_t *)(PERIPHERAL_BITBAND_BASE + ((peripheral_addr - PERIPHERAL_BASE) * 32) + ((bit_number) * 4)) = 0)

#define PERIPHERAL_SET_BIT(peripheral_addr, bit_number)       \
        (*(uint32_t *)(PERIPHERAL_BITBAND_BASE + ((peripheral_addr - PERIPHERAL_BASE) * 32) + ((bit_number) * 4)) = 1)

__IO uint32_t variables, variables_addr = 0, variables_bit_val = 0;

/**
  * @brief  led4 flick for error.
  * @param  none
  * @retval none
  */
void result_error(void)
{
  while(1)
  {
    ht32_led_toggle(LED4);
    delay_sec(1);
  }
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();
  ht32_board_init();
  variables = 0xA5A5A5A5;

  /* get the variables address */
  variables_addr = (uint32_t)&variables;

  /* modify variables bit0 and check result */
  VARIABLES_RESET_BIT(variables_addr, 0);
  if((variables != 0xA5A5A5A4) || (VARIABLES_GET_BIT(variables_addr, 0) != 0))
  {
    result_error();
  }

  /* modify variables bit0 and check result */
  VARIABLES_SET_BIT(variables_addr, 0);
  if((variables != 0xA5A5A5A5) || (VARIABLES_GET_BIT(variables_addr, 0) != 1))
  {
    result_error();
  }

  /* modify variables bit16 and check result */
  VARIABLES_RESET_BIT(variables_addr, 16);
  if((variables != 0xA5A4A5A5) || (VARIABLES_GET_BIT(variables_addr, 16) != 0))
  {
    result_error();
  }

  /* modify variables bit16 and check result */
  VARIABLES_SET_BIT(variables_addr, 16);
  if((variables != 0xA5A5A5A5) || (VARIABLES_GET_BIT(variables_addr, 16) != 1))
  {
    result_error();
  }

  /* modify variables bit31 and check result */
  VARIABLES_RESET_BIT(variables_addr, 31);
  if((variables != 0x25A5A5A5) || (VARIABLES_GET_BIT(variables_addr, 31) != 0))
  {
    result_error();
  }

  /* modify variables bit31 and check result */
  VARIABLES_SET_BIT(variables_addr, 31);
  if((variables != 0xA5A5A5A5) || (VARIABLES_GET_BIT(variables_addr, 31) != 1))
  {
    result_error();
  }

  while(1)
  {
    /* led2 toggle */
    PERIPHERAL_RESET_BIT((uint32_t)&LED2_GPIO->odt, 13);
    delay_ms(500);
    PERIPHERAL_SET_BIT((uint32_t)&LED2_GPIO->odt, 13);
    delay_ms(500);
  }
}

/**
  * @}
  */

/**
  * @}
  */
