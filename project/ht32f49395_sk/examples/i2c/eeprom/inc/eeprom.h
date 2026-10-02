/*********************************************************************************************************//**
 * @file    eeprom.h
 * @version $Rev:: 165         $
 * @date    $Date:: 2025-09-05 #$
 * @brief   i2c eeprom libray header file
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
  
#ifndef __EERPOM_H
#define __EERPOM_H

#include "i2c_application.h"

#define EEPROM_BUSY_TIMEOUT              1000 /*!< eeprom busy waiting timeout */
#define EEPROM_PAGE_SIZE                 8    /*!< eeprom page size */
#define EEPROM_I2C_ADDRESS               0xA0 /*!< eeprom i2c address */

typedef enum
{
  EE_MODE_POLL                           = 0x01, /*!< polling communication */
  EE_MODE_INT                            = 0x02, /*!< interrupt communication */
  EE_MODE_DMA                            = 0x03, /*!< dma communication */
} eeprom_mode_type;

i2c_status_type eeprom_write_buffer(i2c_handle_type* hi2c, eeprom_mode_type mode, i2c_mem_address_width_type mem_address_width, uint16_t address, uint16_t mem_address, uint8_t* pdata, uint16_t size, uint32_t timeout);
i2c_status_type eeprom_read_buffer (i2c_handle_type* hi2c, eeprom_mode_type mode, i2c_mem_address_width_type mem_address_width, uint16_t address, uint16_t mem_address, uint8_t* pdata, uint16_t size, uint32_t timeout);

#endif
