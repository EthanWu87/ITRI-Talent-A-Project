/*********************************************************************************************************//**
 * @file    keyboard_class.h
 * @version $Rev:: 180         $
 * @date    $Date:: 2025-12-09 #$
 * @brief   usb hid keyboard header file
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
#ifndef __KEYBOARD_CLASS_H
#define __KEYBOARD_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_std.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_keyboard_class
  * @{
  */

/** @defgroup USB_keyboard_class_endpoint_definition
  * @{
  */

/**
  * @brief usb hid use endpoint define
  */
#define USBD_KEYBOARD_IN_EPT                  0x81

/**
  * @brief usb hid in and out max packet size define
  */
#define USBD_KEYBOARD_IN_MAXPACKET_SIZE       0x40
#define USBD_KEYBOARD_OUT_MAXPACKET_SIZE      0x40

/**
  * @}
  */


typedef struct
{
  uint32_t hid_protocol;
  uint32_t hid_set_idle;
  uint32_t alt_setting;
  uint8_t hid_set_report[64];
  uint8_t keyboard_buf[8];

  __IO uint8_t hid_suspend_flag;
  __IO uint8_t g_u8tx_completed;
  uint8_t send_state;
  uint8_t hid_state;
  uint8_t temp;

}keyboard_type;

/** @defgroup USB_hid_class_exported_functions
  * @{
  */
extern usbd_class_handler keyboard_class_handler;

usb_sts_type usb_keyboard_class_send_report(void *udev, uint8_t *report, uint16_t len);
void usb_hid_keyboard_send_char(void *udev, uint8_t ascii_code);
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
