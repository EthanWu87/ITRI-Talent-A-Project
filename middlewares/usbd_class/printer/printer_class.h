/*********************************************************************************************************//**
 * @file    printer_class.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb printer class file
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
#ifndef __PRINTER_CLASS_H
#define __PRINTER_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_std.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_printer_class
  * @{
  */

/** @defgroup USB_printer_class_definition
  * @{
  */

#define USBD_PRINTER_BULK_IN_EPT         0x81
#define USBD_PRINTER_BULK_OUT_EPT        0x01


#define USBD_PRINTER_IN_MAXPACKET_SIZE   0x40
#define USBD_PRINTER_OUT_MAXPACKET_SIZE  0x40

#define PRINTER_DEVICE_ID_LEN            24

typedef enum
{
  PRINTER_REQ_GET_DEVICE_ID               = 0x00,
  PRINTER_REQ_GET_PORT_STATUS             = 0x01,
  PRINTER_REQ_GET_SOFT_RESET              = 0x02
}printer_req_type;


typedef struct
{
  uint32_t alt_setting;
  uint32_t g_printer_port_status;
  uint8_t g_rx_buff[USBD_PRINTER_OUT_MAXPACKET_SIZE];
  uint8_t g_printer_data[USBD_PRINTER_OUT_MAXPACKET_SIZE];
  __IO uint8_t g_tx_completed;
  __IO uint8_t g_rx_completed;
  uint32_t g_rxlen;
}printer_type;

extern usbd_class_handler printer_class_handler;
uint16_t usb_printer_get_rxdata(void *udev, uint8_t *recv_data);
error_status usb_printer_send_data(void *udev, uint8_t *send_data, uint16_t len);
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

