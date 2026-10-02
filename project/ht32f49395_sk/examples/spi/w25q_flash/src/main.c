/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 141         $
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
#include "spi_flash.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_SPI_w25q_flash SPI_w25q_flash
  * @{
  */

#define FLASH_TEST_ADDR                  0x1000
#define BUF_SIZE                         0x256

uint8_t tx_buffer[BUF_SIZE];
uint8_t rx_buffer[BUF_SIZE];
volatile error_status transfer_status = ERROR;

void tx_data_fill(void);
error_status buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length);

/**
  * @brief  transfer data fill.
  * @param  none
  * @retval none
  */
void tx_data_fill(void)
{
  uint32_t data_index = 0;
  for(data_index = 0; data_index < BUF_SIZE; data_index++)
  {
    tx_buffer[data_index] = data_index;
  }
}

/**
  * @brief  buffer compare function.
  * @param  pbuffer1, pbuffer2: buffers to be compared.
  * @param  buffer_length: buffer's length
  * @retval the result of compare
  */
error_status buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length)
{
  while(buffer_length--)
  {
    if(*pbuffer1 != *pbuffer2)
    {
      return ERROR;
    }

    pbuffer1++;
    pbuffer2++;
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
  __IO uint32_t index = 0;
  __IO uint32_t flash_id_index = 0;
  system_clock_config();
  ht32_board_init();
  tx_data_fill();
  uart_print_init(115200);
  spiflash_init();
  flash_id_index = spiflash_read_id();
  if((flash_id_index != W25Q128)&&(flash_id_index != W25Q80)&&(flash_id_index != W25Q16)&&(flash_id_index != W25Q32)&&(flash_id_index != W25Q64))
  {
    printf("flash id check error!\r\n");
    for(index = 0; index < 50; index++)
    {
      ht32_led_toggle(LED2);
      ht32_led_toggle(LED3);
      delay_ms(200);
    }
    return 1;
  }
  else
  {
    printf("flash id check success! id: %x\r\n", flash_id_index);
  }

  /* erase sector */
  spiflash_sector_erase(FLASH_TEST_ADDR / SPIF_SECTOR_SIZE);

  /* write data */
  spiflash_write(tx_buffer, FLASH_TEST_ADDR, BUF_SIZE);

  /* read data */
  spiflash_read(rx_buffer, FLASH_TEST_ADDR, BUF_SIZE);

  /* printf read data */
  printf("Read Data: ");
  for(index = 0; index < BUF_SIZE; index++)
  {
    printf("%x ", rx_buffer[index]);
  }

  /* test result:the data check */
  transfer_status = buffer_compare(rx_buffer, tx_buffer, BUF_SIZE);

  /* test result indicate:if SUCCESS ,led2 lights */
  if(transfer_status == SUCCESS)
  {
    printf("\r\nflash data read write success!\r\n");
    ht32_led_on(LED2);
  }
  else
  {
    printf("\r\nflash data read write ERROR!\r\n");
    ht32_led_off(LED2);
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
