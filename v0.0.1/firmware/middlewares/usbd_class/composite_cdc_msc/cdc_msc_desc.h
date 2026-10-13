/*********************************************************************************************************//**
 * @file    cdc_msc_desc.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb cdc descriptor header file
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
#ifndef __CDC_MSC_DESC_H
#define __CDC_MSC_DESC_H

#ifdef __cplusplus
extern "C" {
#endif
 
#include "cdc_msc_class.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */
  
/** @addtogroup USB_cdc_msc_desc
  * @{
  */

/** @defgroup USB_cdc_desc_definition 
  * @{
  */
/**
  * @brief usb bcd number define
  */
#define CDC_BCD_NUM                      0x0110

/**
  * @brief usb vendor id and product id define
  */
#define USBD_CDC_MSC_VENDOR_ID               0x04D9
#define USBD_CDC_MSC_PRODUCT_ID              0x9008

/**
  * @brief usb descriptor size define
  */

#define USBD_CDC_MSC_CONFIG_DESC_SIZE    106
#define USBD_CDC_SIZ_STRING_LANGID       4
#define USBD_CDC_SIZ_STRING_SERIAL       0x1A

/**
  * @brief usb string define(vendor, product configuration, interface)
  */
#define USBD_CDC_DESC_MANUFACTURER_STRING    "Holtek"
#define USBD_CDC_DESC_PRODUCT_STRING         "HT32 Composite VCP and MSC  "
#define USBD_CDC_DESC_CONFIGURATION_STRING   "Composite VCP and MSC Config"
#define USBD_CDC_DESC_INTERFACE_STRING       "Composite VCP and MSC Interface"

/**
  * @brief usb endpoint interval define 
  */
#define CDC_HID_BINTERVAL_TIME                0xFF

/**
  * @brief usb interface define
  */
#define VCPMSC_MSC_INTERFACE                  0x00
#define VCPMSC_CDC_INTERFACE                  0x01
#define VCPMSC_CDC_DATA_INTERFACE             0x02


/**
  * @brief usb mcu id address deine
  */
#define         MCU_ID1                   (0x1FFFF7E8)
#define         MCU_ID2                   (0x1FFFF7EC)
#define         MCU_ID3                   (0x1FFFF7F0)
/**
  * @}
  */
  
extern usbd_desc_handler cdc_msc_desc_handler;


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
