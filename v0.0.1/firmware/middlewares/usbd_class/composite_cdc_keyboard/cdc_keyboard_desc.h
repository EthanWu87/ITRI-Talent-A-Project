/*********************************************************************************************************//**
 * @file    cdc_keyboard_desc.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb cdc and keyboard descriptor header file
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
#ifndef __CDC_KEYBOARD_DESC_H
#define __CDC_KEYBOARD_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cdc_keyboard_class.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_cdc_keyboard_desc
  * @{
  */

/** @defgroup USB_cdc_keyboard_desc_definition
  * @{
  */

/**
  * @brief usb bcd number define
  */
#define VCPKYBRD_BCD_NUM                 0x0110

/**
  * @brief usb vendor id and product id define
  */
#define USBD_VCPKYBRD_VENDOR_ID          0x04D9
#define USBD_VCPKYBRD_PRODUCT_ID         0x9008


/**
  * @brief usb descriptor size define
  */
#define USBD_VCPKYBRD_CONFIG_DESC_SIZE            108
#define USBD_VCPKYBRD_HID_SIZ_REPORT_DESC         63
#define USBD_VCPKYBRD_SIZ_STRING_LANGID           4
#define USBD_VCPKYBRD_SIZ_STRING_SERIAL           0x1A

/**
  * @brief usb string define(vendor, product configuration, interface)
  */
#define USBD_VCPKYBRD_DESC_MANUFACTURER_STRING    "Holtek"
#define USBD_VCPKYBRD_DESC_PRODUCT_STRING         "HT32 Composite VCP and Keyboard"
#define USBD_VCPKYBRD_DESC_CONFIGURATION_STRING   "Composite VCP and Keyboard Config"
#define USBD_VCPKYBRD_DESC_INTERFACE_STRING       "Composite VCP and Keyboard Interface"

/**
  * @brief usb endpoint interval define
  */
#define VCPKYBRD_HID_BINTERVAL_TIME                0x0A

/**
  * @brief usb interface define
  */
#define VCPKYBRD_CDC_INTERFACE                    0x00
#define VCPKYBRD_CDC_DATA_INTERFACE               0x01
#define VCPKYBRD_KEYBOARD_INTERFACE               0x02

/**
  * @brief usb mcu id address deine
  */
#define         MCU_ID1                   (0x1FFFF7E8)
#define         MCU_ID2                   (0x1FFFF7EC)
#define         MCU_ID3                   (0x1FFFF7F0)

/**
  * @}
  */
extern uint8_t g_usbd_vcpkybrd_hid_report[USBD_VCPKYBRD_HID_SIZ_REPORT_DESC];
extern uint8_t g_vcpkybrd_hid_usb_desc[9];
extern usbd_desc_handler cdc_keyboard_desc_handler;


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
