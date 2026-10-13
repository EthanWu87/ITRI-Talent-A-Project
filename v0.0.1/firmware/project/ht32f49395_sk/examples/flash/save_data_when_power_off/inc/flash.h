/*********************************************************************************************************//**
 * @file    flash.h
 * @version $Rev:: 148         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   flash header file
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
#ifndef __FLASH_H__
#define __FLASH_H__

#include "ht32f493x5_board.h"
#include "stdio.h"

/* the select type flash size(the unit is k) */
#define FLASH_SIZE  (*(unsigned int  *)0x1FFFF7E0)
  
/* the flash code size(the unit is k) */
#define FLASH_CODE_SIZE 83
  
/* 0:flash write disable,1:flash write enable */
#define FLASH_WREN 1

uint16_t flash_readhalfword(uint32_t faddr);
void flash_writelenbyte(uint32_t writeaddr,uint32_t datatowrite,uint16_t len);
uint32_t flash_readlenbyte(uint32_t readaddr,uint16_t len);
void flash_write(uint32_t writeaddr,uint16_t *pbuffer,uint16_t numtowrite);
void flash_read(uint32_t readaddr,uint16_t *pbuffer,uint16_t numtoread);

#endif /* __FLASH_H */
