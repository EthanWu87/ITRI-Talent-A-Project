/*********************************************************************************************************//**
 * @file    xmc_psram.h
 * @version $Rev:: 140         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   header file for the psram configuration.
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

/* define to prevent recursive inclusion -------------------------------------*/
#ifndef __XMC_PSRAM_H
#define __XMC_PSRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ht32f493x5_clock.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_XMC_psram
  * @{
  */

/** @defgroup XMC_psram_functions
  * @{
  */

void psram_init(void);
void psram_writebuffer(uint16_t* pbuffer, uint32_t write_addr, uint32_t num_halfword_to_write);
void psram_readbuffer(uint16_t* pbuffer, uint32_t read_addr, uint32_t num_halfword_to_read);

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

