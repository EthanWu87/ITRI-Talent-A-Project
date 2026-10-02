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
#include "flash.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_FLASH_write_read FLASH_write_read
  * @{
  */

#define TEST_BUFEER_SIZE                 3000
#define TEST_FLASH_ADDRESS_START         (0x08000000 + 1024 * 10)

uint16_t buffer_write[TEST_BUFEER_SIZE];
uint16_t buffer_read[TEST_BUFEER_SIZE];

error_status buffer_compare(uint16_t* p_buffer1, uint16_t* p_buffer2, uint16_t buffer_length);

/**
  * @brief  compares two buffers.
  * @param  p_buffer1, p_buffer2: buffers to be compared.
  * @param  buffer_length: buffer's length
  * @retval SUCCESS: p_buffer1 identical to p_buffer2
  *         ERROR: p_buffer1 differs from p_buffer2
  */
error_status buffer_compare(uint16_t* p_buffer1, uint16_t* p_buffer2, uint16_t buffer_length)
{
  while(buffer_length--)
  {
    if(*p_buffer1 != *p_buffer2)
    {
      return ERROR;
    }
    p_buffer1++;
    p_buffer2++;
  }
  return SUCCESS;
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  uint32_t index=0;
  error_status err_status;
  system_clock_config();
  ht32_board_init();
  /* fill buffer_write data to test */
  for(index = 0; index < TEST_BUFEER_SIZE; index++)
  {
    buffer_write[index] = index;
  }

  /* write data to flash */
  err_status = flash_write(TEST_FLASH_ADDRESS_START, buffer_write, TEST_BUFEER_SIZE);

  /* read data from flash */
  flash_read(TEST_FLASH_ADDRESS_START, buffer_read, TEST_BUFEER_SIZE);

  /* compare the buffer */
  if((buffer_compare(buffer_write, buffer_read, TEST_BUFEER_SIZE) == SUCCESS) && (err_status == SUCCESS))
  {
    ht32_led_on(LED2);
    ht32_led_on(LED3);
    ht32_led_on(LED4);
  }

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
