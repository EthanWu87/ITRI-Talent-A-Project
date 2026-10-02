/*********************************************************************************************************//**
 * @file    custom_hid_desc.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb custom hid descriptor header file
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
#ifndef __CUSTOM_HID_DESC_H
#define __CUSTOM_HID_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "custom_hid_class.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_custom_hid_desc
  * @{
  */

/** @defgroup USB_custom_hid_desc_definition
  * @{
  */


/**
  * @brief usb bcd number define
  */
#define CUSHID_BCD_NUM                   0x0110

/**
  * @brief usb vendor id and product id define
  */
#define USBD_CUSHID_VENDOR_ID            0x04D9
#define USBD_CUSHID_PRODUCT_ID           0x9008

/**
  * @brief usb descriptor size define
  */
#define USBD_CUSHID_CONFIG_DESC_SIZE     41
#define USBD_CUSHID_SIZ_REPORT_DESC      126
#define USBD_CUSHID_SIZ_STRING_LANGID    4
#define USBD_CUSHID_SIZ_STRING_SERIAL    0x1A

/**
  * @brief usb string define(vendor, product configuration, interface)
  */
#define USBD_CUSHID_DESC_MANUFACTURER_STRING    "Holtek"
#define USBD_CUSHID_DESC_PRODUCT_STRING         "Custom HID"
#define USBD_CUSHID_DESC_CONFIGURATION_STRING   "Custom HID Config"
#define USBD_CUSHID_DESC_INTERFACE_STRING       "Custom HID Interface"

#define CUSHID_BINTERVAL_TIME            0x0A

/**
  * @brief usb hid report id define
  */
/**
  * @brief usb hid report id define
  */
#define HID_REPORT_ID_1                   0x01
#define HID_REPORT_ID_2                   0x02
#define HID_REPORT_ID_3                   0x03
#define HID_REPORT_ID_4                   0x04
#define HID_REPORT_ID_5                   0x05
#define HID_REPORT_ID_6                   0xF0

/**
  * @brief usb mcu id address deine
  */
#define MCU_ID1                          (0x1FFFF7E8)
#define MCU_ID2                          (0x1FFFF7EC)
#define MCU_ID3                          (0x1FFFF7F0)

/**
  * @}
  */
extern uint8_t g_usbd_custom_hid_report[USBD_CUSHID_SIZ_REPORT_DESC];
extern uint8_t g_custom_hid_usb_desc[9];
extern usbd_desc_handler custom_hid_desc_handler;

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


