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

#include "xmc_ecc.h"
#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_XMC_nand_ecc XMC_nand_ecc
  * @{
  */

crm_clocks_freq_type crm_clocks_freq_struct = {0};
nand_id_type nand_id_struct;
nand_address_type write_read_addr_struct;
uint8_t tx_buffer[BUFFER_SIZE], rx_buffer[BUFFER_SIZE];
uint32_t page_number = 1, status = 0;
uint32_t j = 0, ecc_value_write = 0, ecc_value_read = 0, ecc_value_write_last = 0;

void fill_buffer(uint8_t *pbuffer, uint16_t bufferlenght, uint32_t offset);
void nand_ecc_correction(uint8_t *pbuffer, uint32_t tx_ecc_value, uint32_t rx_ecc_value);

/**
  * @brief  Fill the global buffer
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferSize: size of the buffer to fill
  * @param  Offset: first value to fill on the Buffer
  */
void fill_buffer(uint8_t *pbuffer, uint16_t bufferlenght, uint32_t offset)
{
  uint16_t indextmp = 0;

  /* Put in global buffer same values */
  for(indextmp = 0; indextmp < bufferlenght; indextmp++ )
  {
    pbuffer[indextmp] = indextmp + offset;
  }
}

/**
  * @brief  ecc correct 1 bit error
  * @param  pbuffer: pointer on the buffer to correct
  * @param  tx_ecc_value: ecc value for transmiting to nand flash
  * @param  rx_ecc_value: ecc value for receiving from nand flash
  */
void nand_ecc_correction(uint8_t *pbuffer,uint32_t tx_ecc_value ,uint32_t rx_ecc_value)
{
  uint32_t ecc_value=0, position=0 ,byte_position=0;
  uint8_t i,compare_data;

  /* check ecc value */
  if(tx_ecc_value!=rx_ecc_value)
  {
    /* 2048 byte -- 28bit ecc valid value; 8192 byte -- 32bit ecc valid value */
#ifdef H27U1G8F2CTR
    ecc_value =(tx_ecc_value^rx_ecc_value)&0x0FFFFFFF;
#elif defined K9GAG08U0E
    ecc_value =(tx_ecc_value^rx_ecc_value)&0xFFFFFFFF;
#endif
    /* 1 bit error correction */
#ifdef H27U1G8F2CTR
    for(i=0;i<(28/2);i++)
#elif defined K9GAG08U0E
    for(i=0;i<(32/2);i++)
#endif
    {
      compare_data = (ecc_value>>(i*2))&0x3;

      /* find position */
      if(compare_data==0x2)
      {
        position |= (1<<i);
      }

      /* more than 1 bit erroc */
      else if(compare_data!=0x1)
      {
        return;
      }
    }

    /* correct receive value */
    byte_position = position/8;
    pbuffer[byte_position] ^= 1 << (position % 8);
  }
}

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
  delay_us(10);

  /* nand read id command */
  nand_read_id(&nand_id_struct);

  /* verify the nand id */
  /* nand support:spansion:s34ml01g100tfi003  micron:mt29f16g08cbaca  sandisk:sdtnqgama-008g  toshiba:tc58teg6ddkta00   samsung:k9gag08u0e-scb0   hynix:h27u1g8f2ctr
                  id      :0x01f1001d         id    :0x2c48044a       id     :0x45de9493      id     :0x98de9493        id     :0xecd58472        id   :0xadf1801d
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

    /* write data to xmc nand memory */
    /* fill the buffer to send */
    fill_buffer(tx_buffer, BUFFER_SIZE , 0x0);

    /* change the regular to make ecc value isn't 0 */
    tx_buffer[10]=0x03;

     /* calculate ecc value while transmiting */
    xmc_nand_ecc_enable(XMC_BANK2_NAND, TRUE);
    status = nand_write_small_page(tx_buffer, write_read_addr_struct, page_number);

    /* save the right ecc_value (in this case ,we suppose it as the true value) */
    ecc_value_write_last = ecc_value_write;

    /* change the 85 data like there is a 1 bit error happened */
    status = nand_erase_block(write_read_addr_struct);
    tx_buffer[10]=0x23;
    status = nand_write_small_page(tx_buffer, write_read_addr_struct, page_number);

    /* calculate ecc value while transmiting */
    xmc_nand_ecc_enable(XMC_BANK2_NAND, TRUE);

    /* read back the written data */
    status = nand_read_small_page (rx_buffer, write_read_addr_struct, page_number);

    /* ecc check and correct 1 bit error */
    nand_ecc_correction(rx_buffer ,ecc_value_write_last ,ecc_value_read);

    if(rx_buffer[10]==0x03)
    {
      printf("ecc is work\r\n");
    }
    else
    {
      printf("ecc is not work\r\n");
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
  * @}
  */

/**
  * @}
  */
