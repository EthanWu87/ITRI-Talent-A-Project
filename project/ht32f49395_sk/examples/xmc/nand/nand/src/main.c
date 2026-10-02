/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 140         $
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

#include "xmc_nand.h"
#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_XMC_nand XMC_nand
  * @{
  */

crm_clocks_freq_type crm_clocks_freq_struct = {0};
nand_id_type nand_id_struct = {0};
nand_address_type write_read_addr_struct;
uint8_t txbuffer[BUFFER_SIZE], rxbuffer[BUFFER_SIZE + NAND_SPARE_AREA_SIZE];
uint32_t pagenumber = BUFFER_SIZE / NAND_PAGE_SIZE, writereadstatus = 0, status= 0;
uint32_t j = 0;

void fill_buffer(uint8_t *pbuffer, uint16_t bufferlenght, uint32_t offset);

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* configure the system clock */
  system_clock_config();

  /* led initalization */
  ht32_board_init();

  /* usart initalization */
  uart_print_init(115200);

  /* get system clock */
  crm_clocks_freq_get(&crm_clocks_freq_struct);

  /* xmc initialization */
  nand_init();

  /* nand reset command */
  nand_reset();
  delay_ms(1);

  /* nand read id command */
  nand_read_id(&nand_id_struct);

  /* verify the nand id */
  /* nand support:samsung:k9gag08u0e        hynix:h27u1g8f2ctr
                  id     :0xecd58472        id   :0xadf1801d
  */
  if((nand_id_struct.maker_id == NAND_AT_MakerID) && (nand_id_struct.device_id == NAND_AT_DeviceID))
  {

    /* nand memory address to write to */
    write_read_addr_struct.zone  = 0x00;
    write_read_addr_struct.block = 0x00;
    write_read_addr_struct.page  = 0x00;
    write_read_addr_struct.byte  = 0x00;

    /* erase the nand first block */
    status = nand_erase_block(write_read_addr_struct);

    /* fill the buffer to send */
    fill_buffer(txbuffer, BUFFER_SIZE , 0x66);

    /* write data to xmc nand memory */
    status = nand_write_small_page(txbuffer, write_read_addr_struct, pagenumber);

    /* read back the written data */
    status = nand_read_small_page (rxbuffer, write_read_addr_struct, pagenumber);

    /* verify the written data */
    for(j = 0; j < BUFFER_SIZE; j++)
    {
      if(txbuffer[j] != rxbuffer[j])
      {
        writereadstatus++;
      }
    }

    if(writereadstatus == 0)
    {
      printf("data is right\r\n");
    }
    else
    {
      printf("data is error\r\n");
    }
  }
  else
  {
    printf("the id is error\r\n");
  }

  while(1)
  {
  }
}

/**
  * @brief  fill the global buffer
  * @param  pbuffer: pointer on the buffer to fill
  * @param  buffersize: size of the buffer to fill
  * @param  offset: first value to fill on the buffer
  */
void fill_buffer(uint8_t *pbuffer, uint16_t bufferlenght, uint32_t offset)
{
  uint16_t indextmp = 0;

  /* put in global buffer same values */
  for(indextmp = 0; indextmp < bufferlenght; indextmp++ )
  {
    pbuffer[indextmp] = indextmp + offset;
  }
}

/**
  * @}
  */

/**
  * @}
  */

