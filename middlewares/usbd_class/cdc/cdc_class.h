/*********************************************************************************************************//**
 * @file    cdc_class.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb cdc class file
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
#ifndef __CDC_CLASS_H
#define __CDC_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_std.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_cdc_class
  * @{
  */

/** @defgroup USB_cdc_class_definition
  * @{
  */

/**
  * @brief usb cdc use endpoint define
  */
#define USBD_CDC_INT_EPT                 0x82
#define USBD_CDC_BULK_IN_EPT             0x81
#define USBD_CDC_BULK_OUT_EPT            0x01

/**
  * @brief usb cdc in and out max packet size define
  */
#define USBD_CDC_IN_MAXPACKET_SIZE        0x40
#define USBD_CDC_OUT_MAXPACKET_SIZE       0x40
#define USBD_CDC_CMD_MAXPACKET_SIZE       0x08

/**
  * @}
  */

/** @defgroup USB_cdc_class_exported_types
  * @{
  */

/**
  * @brief usb cdc class struct
  */
typedef struct
{
  uint32_t alt_setting;
  uint8_t g_rx_buff[USBD_CDC_OUT_MAXPACKET_SIZE];
  uint8_t g_cmd[USBD_CDC_CMD_MAXPACKET_SIZE];
  uint8_t g_req;
  uint16_t g_len, g_rxlen;
  __IO uint8_t g_tx_completed, g_rx_completed;
  linecoding_type linecoding;
}cdc_struct_type;


/**
  * @}
  */

/** @defgroup USB_cdc_class_exported_functions
  * @{
  */
extern usbd_class_handler cdc_class_handler;
uint16_t usb_vcp_get_rxdata(void *udev, uint8_t *recv_data);
error_status usb_vcp_send_data(void *udev, uint8_t *send_data, uint16_t len);

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



