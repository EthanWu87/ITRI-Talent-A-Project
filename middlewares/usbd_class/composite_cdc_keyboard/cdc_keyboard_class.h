/*********************************************************************************************************//**
 * @file    cdc_keyboard_class.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb cdc and keyboard class file
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
#ifndef __CDC_KEYBOARD_CLASS_H
#define __CDC_KEYBOARD_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_std.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_cdc_keyboard_class
  * @{
  */

/** @defgroup USB_cdc_keyboard_class_definition
  * @{
  */

/**
  * @brief usb use endpoint define
  */
#define USBD_VCPKYBRD_CDC_INT_EPT                 0x82
#define USBD_VCPKYBRD_CDC_BULK_IN_EPT             0x81
#define USBD_VCPKYBRD_CDC_BULK_OUT_EPT            0x01
#define USBD_VCPKYBRD_HID_IN_EPT                  0x83

/**
  * @brief usb in and out max packet size define
  */
#define USBD_VCPKYBRD_IN_MAXPACKET_SIZE           0x40
#define USBD_VCPKYBRD_OUT_MAXPACKET_SIZE          0x40
#define USBD_VCPKYBRD_CMD_MAXPACKET_SIZE          0x08


/**
  * @}
  */

/** @defgroup USB_cdc_class_exported_types
  * @{
  */

typedef struct
{
  uint32_t alt_setting;
  uint32_t hid_protocol;
  uint32_t hid_set_idle;

  uint8_t hid_set_report[64];
  uint8_t g_rx_buff[USBD_VCPKYBRD_OUT_MAXPACKET_SIZE];
  uint8_t g_cmd[USBD_VCPKYBRD_CMD_MAXPACKET_SIZE];
  uint8_t g_req;
  uint8_t hid_state;
  uint16_t g_len, g_rxlen;
  __IO uint8_t g_tx_completed;
  __IO uint8_t g_rx_completed;
  __IO uint8_t g_keyboard_tx_completed;

  linecoding_type linecoding;
}vcp_keyboard_type;

/**
  * @}
  */

/** @defgroup USB_cdc_keyboar_class_exported_functions
  * @{
  */
extern usbd_class_handler cdc_keyboard_class_handler;
uint16_t usb_vcpkybrd_vcp_get_rxdata(void *udev, uint8_t *recv_data);
error_status usb_vcpkybrd_vcp_send_data(void *udev, uint8_t *send_data, uint16_t len);
usb_sts_type usb_vcpkybrd_class_send_report(void *udev, uint8_t *report, uint16_t len);
void usb_vcpkybrd_keyboard_send_char(void *udev, uint8_t ascii_code);

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




