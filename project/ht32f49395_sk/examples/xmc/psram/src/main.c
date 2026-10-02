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

#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"
#include "xmc_psram.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_XMC_psram XMC_psram
  * @{
  */

#define BUFFER_SIZE        0x400
#define WRITE_READ_ADDR    0x8000

uint16_t txbuffer[BUFFER_SIZE];
uint16_t rxbuffer[BUFFER_SIZE];
uint32_t writereadstatus = 0;
uint16_t index_cnt = 0;

void fill_buffer(uint16_t *pBuffer, uint16_t BufferLenght, uint32_t Offset);

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* configure the system clock */
  system_clock_config();

  /* initialize leds */
  ht32_board_init();

  /* initialize usart */
  uart_print_init(115200);

  /* configure xmc bank1 nor/sram1 */
  psram_init();

  /* fill the buffer to send */
  fill_buffer(txbuffer, BUFFER_SIZE, 0x3212);

  /* write data to xmc sram memory */
  psram_writebuffer(txbuffer, WRITE_READ_ADDR, BUFFER_SIZE);

  /* read data from xmc sram memory */
  psram_readbuffer(rxbuffer, WRITE_READ_ADDR, BUFFER_SIZE);

  /* read back sram memory and check content correctness */
  for(index_cnt = 0x00; (index_cnt < BUFFER_SIZE) && (writereadstatus == 0); index_cnt++)
  {
    if(rxbuffer[index_cnt] != txbuffer[index_cnt])
    {
      writereadstatus = index_cnt + 1;
    }
  }

  if(writereadstatus == 0)
  {
    /* pass */
    printf("data is right\r\n");
  }
  else
  {
    /* fail */
    printf("data is error\r\n");
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
void fill_buffer(uint16_t *pbuffer, uint16_t bufferlenght, uint32_t offset)
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

