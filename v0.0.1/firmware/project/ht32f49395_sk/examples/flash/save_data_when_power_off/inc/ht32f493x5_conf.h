/*********************************************************************************************************//**
 * @file    ht32f493x5_conf.h
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   ht32f493x5 config header file
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
#ifndef __HT32F493x5_CONF_H
#define __HT32F493x5_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief in the following line adjust the value of high speed external crystal (hext)
  * used in your application
  *
  * tip: to avoid modifying this file each time you need to use different hext, you
  *      can define the hext value in your toolchain compiler preprocessor.
  *
  */
#if !defined  HEXT_VALUE
#define HEXT_VALUE               ((uint32_t)8000000) /*!< value of the high speed external crystal in hz */
#endif

/**
  * @brief in the following line adjust the high speed external crystal (hext) startup
  * timeout value
  */
#define HEXT_STARTUP_TIMEOUT             ((uint16_t)0x3000)  /*!< time out for hext start up */
#define HICK_VALUE                       ((uint32_t)8000000) /*!< value of the high speed internal clock in hz */
#define LEXT_VALUE                       ((uint32_t)32768)   /*!< value of the low speed external clock in hz */

/* module define -------------------------------------------------------------*/
#define CRM_MODULE_ENABLED
#define TMR_MODULE_ENABLED
#define RTC_MODULE_ENABLED
#define BPR_MODULE_ENABLED
#define GPIO_MODULE_ENABLED
#define I2C_MODULE_ENABLED
#define USART_MODULE_ENABLED
#define PWC_MODULE_ENABLED
#define CAN_MODULE_ENABLED
#define ADC_MODULE_ENABLED
#define DAC_MODULE_ENABLED
#define SPI_MODULE_ENABLED
#define DMA_MODULE_ENABLED
#define DEBUG_MODULE_ENABLED
#define FLASH_MODULE_ENABLED
#define CRC_MODULE_ENABLED
#define WWDT_MODULE_ENABLED
#define WDT_MODULE_ENABLED
#define EXINT_MODULE_ENABLED
#define SDIO_MODULE_ENABLED
#define XMC_MODULE_ENABLED
#define USB_MODULE_ENABLED
#define ACC_MODULE_ENABLED
#define MISC_MODULE_ENABLED

/* includes ------------------------------------------------------------------*/
#ifdef CRM_MODULE_ENABLED
#include "ht32f493x5_crm.h"
#endif
#ifdef TMR_MODULE_ENABLED
#include "ht32f493x5_tmr.h"
#endif
#ifdef RTC_MODULE_ENABLED
#include "ht32f493x5_rtc.h"
#endif
#ifdef BPR_MODULE_ENABLED
#include "ht32f493x5_bpr.h"
#endif
#ifdef GPIO_MODULE_ENABLED
#include "ht32f493x5_gpio.h"
#endif
#ifdef I2C_MODULE_ENABLED
#include "ht32f493x5_i2c.h"
#endif
#ifdef USART_MODULE_ENABLED
#include "ht32f493x5_usart.h"
#endif
#ifdef PWC_MODULE_ENABLED
#include "ht32f493x5_pwc.h"
#endif
#ifdef CAN_MODULE_ENABLED
#include "ht32f493x5_can.h"
#endif
#ifdef ADC_MODULE_ENABLED
#include "ht32f493x5_adc.h"
#endif
#ifdef DAC_MODULE_ENABLED
#include "ht32f493x5_dac.h"
#endif
#ifdef SPI_MODULE_ENABLED
#include "ht32f493x5_spi.h"
#endif
#ifdef DMA_MODULE_ENABLED
#include "ht32f493x5_dma.h"
#endif
#ifdef DEBUG_MODULE_ENABLED
#include "ht32f493x5_debug.h"
#endif
#ifdef FLASH_MODULE_ENABLED
#include "ht32f493x5_flash.h"
#endif
#ifdef CRC_MODULE_ENABLED
#include "ht32f493x5_crc.h"
#endif
#ifdef WWDT_MODULE_ENABLED
#include "ht32f493x5_wwdt.h"
#endif
#ifdef WDT_MODULE_ENABLED
#include "ht32f493x5_wdt.h"
#endif
#ifdef EXINT_MODULE_ENABLED
#include "ht32f493x5_exint.h"
#endif
#ifdef SDIO_MODULE_ENABLED
#include "ht32f493x5_sdio.h"
#endif
#ifdef XMC_MODULE_ENABLED
#include "ht32f493x5_xmc.h"
#endif
#ifdef ACC_MODULE_ENABLED
#include "ht32f493x5_acc.h"
#endif
#ifdef MISC_MODULE_ENABLED
#include "ht32f493x5_misc.h"
#endif
#ifdef USB_MODULE_ENABLED
#include "ht32f493x5_usb.h"
#endif

#ifdef __cplusplus
}
#endif

#endif
