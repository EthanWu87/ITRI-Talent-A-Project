/*********************************************************************************************************//**
 * @file    custom_hid_class.h
 * @version $Rev:: 180         $
 * @date    $Date:: 2025-12-09 #$
 * @brief   usb custom hid header file
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
#ifndef __CUSTOM_HID_CLASS_H
#define __CUSTOM_HID_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_std.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_custom_hid_class
  * @{
  */

/** @defgroup USB_custom_hid_class_endpoint_definition
  * @{
  */

/**
  * @brief usb custom hid use endpoint define
  */
#define USBD_CUSTOM_HID_IN_EPT                  0x81
#define USBD_CUSTOM_HID_OUT_EPT                 0x01

/**
  * @brief usb custom hid in and out max packet size define
  */
#define USBD_CUSTOM_IN_MAXPACKET_SIZE           0x40
#define USBD_CUSTOM_OUT_MAXPACKET_SIZE          0x40

/**
  * @}
  */

/** @defgroup USB_custom_hid_class_request_code_definition
  * @{
  */

typedef struct
{
  uint8_t g_rxhid_buff[USBD_CUSTOM_OUT_MAXPACKET_SIZE];
  uint8_t g_txhid_buff[USBD_CUSTOM_IN_MAXPACKET_SIZE];

  uint32_t hid_protocol;
  uint32_t hid_set_idle;
  uint32_t alt_setting;
  
  uint8_t hid_set_report[64];
  uint8_t hid_get_report[64];
  uint8_t hid_state;
  uint8_t send_state;
}custom_hid_type;

/**
  * @}
  */

/** @defgroup USB_custom_hid_class_exported_functions
  * @{
  */
extern usbd_class_handler custom_hid_class_handler;
usb_sts_type custom_hid_class_send_report(void *udev, uint8_t *report, uint16_t len);
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
