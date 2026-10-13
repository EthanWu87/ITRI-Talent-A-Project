/*********************************************************************************************************//**
 * @file    flash_fat16.h
 * @version $Rev:: 135         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   fat16 file system header file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_FAT16_H
#define __FLASH_FAT16_H

#ifdef __cplusplus
extern "C" {
#endif


#include "usb_conf.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_USB_device_virtual_msc_iap
  * @{
  */

/** @defgroup USB_device_virtual_msc_iap_definition
  * @{
  */
#define FAT16_SECTOR_SIZE                62
#define FAT16_TABLE_SIZE                 4
#define FAT16_BYTE_PER_SIZE              2048
#define FAT16_FILENAME_SIZE              11
#define FAT16_DIR_SIZE                   47

#define FLASH_FAT16_BOOT_SECTOR_ADDR     0x00
#define FLASH_FAT16_1_ADDR               0x3000
#define FLASH_FAT16_2_ADDR               0x3800
#define FLASH_FAT16_ROOT_ADDR            0x4000
#define FLASH_FAT16_FILE_START_ADDR      0x8800

#define FLASH_APP_START_ADDR             0x08005000

#define FLASH_SECTOR_1K_SIZE             0x400
#define FLASH_SECTOR_2K_SIZE             0x800
#define FLASH_SECTOR_4K_SIZE             0x1000

#define FLASH_SECTOR_1K_ALLGNED          0x3FF
#define FLASH_SECTOR_2K_ALLGNED          0x7FF
#define FLASH_SECTOR_4K_ALLGNED          0xFFF

#define FILE_SUFFIX1_LEN                 3
#define FILE_SUFFIX2_LEN                 3
#define FILE_SUFFIX1_NAME                "BIN"
#define FILE_SUFFIX2_NAME                "bin"

#define INVAILD_OFFSET_ADDR              0xFFFFFFFF
#define IAP_UPGRADE_COMPLETE_FLAG        0x41544B38
#define IAP_SUCCESS                      0
#define IAP_FAILED                       1

typedef enum
{
  UPGRADE_READY                          = 0,
  UPGRAGE_ONGOING,
  UPGRADE_SUCCESS,
  UPGRADE_FAILED,
  UPGRADE_LARGE,
  UPGRADE_UNKNOWN,
  UPGRADE_DONE,
  UPGRADE_JUMP
}upgrade_status_type;

typedef struct
{
  uint8_t file_name[11];
  uint8_t attr;
  uint8_t res;
  uint8_t create_time_tenth;
  uint16_t create_time;
  uint16_t create_data;
  uint16_t last_access_data;
  uint16_t clus_high;
  uint16_t write_time;
  uint16_t write_data;
  uint16_t clus_low;
  uint32_t file_size;
}fat_dir_type;

typedef struct
{
  uint32_t flash_base_addr;
  uint32_t flash_app_addr;
  uint32_t flash_size;
  uint32_t flash_app_size;
  uint32_t sector_size;
  uint32_t sector_mask;
  uint32_t firmware_length;
  uint32_t write_crc;
  uint32_t read_crc;

  uint32_t write_addr;
  uint32_t file_write_nr;
  upgrade_status_type msc_up_status;
}flash_iap_type;

extern flash_iap_type flash_iap;

uint32_t flash_fat16_write(uint32_t fat_lbk, uint8_t *data, uint32_t len);
uint32_t flash_fat16_read(uint32_t fat_lbk, uint8_t *data, uint32_t len);

void flash_fat16_init(void);
void flash_fat16_loop_status(void);
uint32_t flash_fat16_set_name(const uint8_t *file_name, uint8_t len);
uint8_t flash_fat16_get_upgrade_flag(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif

