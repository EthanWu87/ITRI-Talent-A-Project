/*********************************************************************************************************//**
 * @file    keyboard_desc.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb keyboard descriptor header file
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
#ifndef __KEYBOARD_DESC_H
#define __KEYBOARD_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "keyboard_class.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_keyboard_desc
  * @{
  */

/** @defgroup USB_keyboard_desc_definition
  * @{
  */

/**
  * @brief usb bcd number define
  */
#define KEYBOARD_BCD_NUM                      0x0110

/**
  * @brief usb vendor id and product id define
  */
#define USBD_KEYBOARD_VENDOR_ID          0x04D9
#define USBD_KEYBOARD_PRODUCT_ID         0x9008

/**
  * @brief usb descriptor size define
  */
#define USBD_KEYBOARD_CONFIG_DESC_SIZE    34
#define USBD_KEYBOARD_SIZ_REPORT_DESC     63
#define USBD_KEYBOARD_SIZ_STRING_LANGID   4
#define USBD_KEYBOARD_SIZ_STRING_SERIAL   0x1A

/**
  * @brief usb string define(vendor, product configuration, interface)
  */
#define USBD_KEYBOARD_DESC_MANUFACTURER_STRING    "Holtek"
#define USBD_KEYBOARD_DESC_PRODUCT_STRING         "Keyboard"
#define USBD_KEYBOARD_DESC_CONFIGURATION_STRING   "Keyboard Config"
#define USBD_KEYBOARD_DESC_INTERFACE_STRING       "Keyboard Interface"

/**
  * @brief usb hid endpoint interval define
  */
#define KEYBOARD_BINTERVAL_TIME                0x0A

/**
  * @brief usb mcu id address deine
  */
#define         MCU_ID1                   (0x1FFFF7E8)
#define         MCU_ID2                   (0x1FFFF7EC)
#define         MCU_ID3                   (0x1FFFF7F0)

/**
  * @}
  */
extern uint8_t g_usbd_keyboard_report[USBD_KEYBOARD_SIZ_REPORT_DESC];
extern uint8_t g_keyboard_usb_desc[9];
extern usbd_desc_handler keyboard_desc_handler;

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


