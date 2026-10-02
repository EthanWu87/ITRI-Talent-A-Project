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

#include <stdbool.h>
#include <string.h>
#include "ht32_sdio.h"
#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"
#include "ff.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_SDIO_sdio_fatfs SDIO_sdio_fatfs
  * @{
  */

FATFS fs;
FIL file;
BYTE work[FF_MAX_SS];

uint8_t buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length);
static void sd_test_error(void);
static void nvic_configuration(void);

/**
  * @brief  compares two buffers.
  * @param  pbuffer1, pbuffer2: buffers to be compared.
  * @param  buffer_length: buffer's length
  * @retval 1: pbuffer1 identical to pbuffer2
  *         0: pbuffer1 differs from pbuffer2
  */
uint8_t buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length)
{
  while(buffer_length--)
  {
    if(*pbuffer1 != *pbuffer2)
    {
      return 0;
    }
    pbuffer1++;
    pbuffer2++;
  }
  return 1;
}

/**
  * @brief  led2 on off every 300ms for sd test failed.
  * @param  none
  * @retval none
  */
static void sd_test_error(void)
{
  ht32_led_on(LED2);
  delay_ms(300);
  ht32_led_off(LED2);
  delay_ms(300);
}

/**
  * @brief  configures sdio1 irq channel.
  * @param  none
  * @retval none
  */
static void nvic_configuration(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(SDIO1_IRQn, 0, 0);
}

/**
  * @brief  fatfs file read/write test.
  * @param  none
  * @retval ERROR: fail.
  *         SUCCESS: success.
  */
static error_status fatfs_test(void)
{
  FRESULT ret;
  char filename[] = "1:/test1.txt";
  const char wbuf[] = "this is my file for test fatfs!\r\n";
  char rbuf[50];
  UINT bytes_written = 0;
  UINT bytes_read = 0;
  DWORD fre_clust, fre_sect, tot_sect;
  FATFS* pt_fs;

  ret = f_mount(&fs, "1:", 1);
  if(ret){
    printf("fs mount err:%d.\r\n", ret);
    if(ret == FR_NO_FILESYSTEM){
      printf("create fatfs..\r\n");
      ret = f_mkfs("1:", 0, work, sizeof(work));
      if(ret){
        printf("creates fatfs err:%d.\r\n", ret);
        return ERROR;
      }
      else{
        printf("creates fatfs ok.\r\n");
      }
      ret = f_mount(NULL, "1:", 1);
      ret = f_mount(&fs, "1:", 1);
      if(ret){
        printf("fs mount err:%d.\r\n", ret);
        return ERROR;
      }
      else{
        printf("fs mount ok.\r\n");
      }
    }
    else{
      return ERROR;
    }
  }
  else{
    printf("fs mount ok.\r\n");
  }

  ret = f_open(&file, filename, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
  if(ret){
    printf("open file err:%d.\r\n", ret);
  }
  else{
    printf("open file ok.\r\n");
  }
  ret = f_write(&file, wbuf, sizeof(wbuf), &bytes_written);
  if(ret){
    printf("write file err:%d.\r\n", ret);
  }
  else{
    printf("write file ok, byte:%u.\r\n", bytes_written);
  }
  f_lseek(&file, 0);
  ret = f_read(&file, rbuf, sizeof(rbuf), &bytes_read);
  if(ret){
    printf("read file err:%d.\r\n", ret);
  }
  else{
    printf("read file ok, byte:%u.\r\n", bytes_read);
  }
  ret = f_close(&file);
  if(ret){
    printf("close file err:%d.\r\n", ret);
  }
  else{
    printf("close file ok.\r\n");
  }

  pt_fs = &fs;
  /* get volume information and free clusters of drive 1 */
  ret = f_getfree("1:", &fre_clust, &pt_fs);
  if(ret == FR_OK)
  {
    /* get total sectors and free sectors */
    tot_sect = (pt_fs->n_fatent - 2) * pt_fs->csize;
    fre_sect = fre_clust * pt_fs->csize;

    /* print the free space (assuming 512 bytes/sector) */
    printf("%10u KiB total drive space.\r\n%10u KiB available.\r\n", tot_sect / 2, fre_sect / 2);
  }

  ret = f_mount(NULL, "1:", 1);

  if(1 == buffer_compare((uint8_t*)rbuf, (uint8_t*)wbuf, sizeof(wbuf))){
    printf("r/w file data test ok.\r\n");
  }
  else{
    printf("r/w file data test fail.\r\n");
    return ERROR;
  }

  return SUCCESS;
}

/* gloable functions ---------------------------------------------------------*/
/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  ht32_board_init();

  nvic_configuration();

  uart_print_init(115200);
  printf("start test fatfs r0.14b..\r\n");

  if(SUCCESS != fatfs_test())
  {
    while(1)
    {
      sd_test_error();
    }
  }

  /* all tests pass, led3 and led4 fresh */
  while(1)
  {
    ht32_led_toggle(LED3);
    ht32_led_toggle(LED4);
    delay_ms(300);
  }
}

/**
  * @}
  */

/**
  * @}
  */
