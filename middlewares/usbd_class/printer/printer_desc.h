/*********************************************************************************************************//**
 * @file    printer_desc.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb printer descriptor header file
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
#ifndef __PRINTER_DESC_H
#define __PRINTER_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "printer_class.h"
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_printer_desc
  * @{
  */

/** @defgroup USB_printer_desc_definition
  * @{
  */

#define PRINTER_BCD_NUM                  0x0110

#define USBD_PRINTER_VENDOR_ID           0x04D9
#define USBD_PRINTER_PRODUCT_ID          0x9008

#define USBD_PRINTER_CONFIG_DESC_SIZE    32
#define USBD_PRINTER_SIZ_STRING_LANGID   4
#define USBD_PRINTER_SIZ_STRING_SERIAL   0x1A

#define USBD_PRINTER_DESC_MANUFACTURER_STRING    "Holtek"
#define USBD_PRINTER_DESC_PRODUCT_STRING         "HT32 Printer"
#define USBD_PRINTER_DESC_CONFIGURATION_STRING   "Printer Config"
#define USBD_PRINTER_DESC_INTERFACE_STRING       "Printer Interface"

#define PRINTER_PROTOCOL_UNIDIRECTIONAL   0x01
#define PRINTER_PROTOCOL_BI_DIRECTIONAL   0x02
#define PRINTER_PROTOCOL_1284_4           0x03
#define PRINTER_PROTOCOL_VENDOR_SPECIFIC  0xFF


#define         MCU_ID1                   (0x1FFFF7E8)
#define         MCU_ID2                   (0x1FFFF7EC)
#define         MCU_ID3                   (0x1FFFF7F0)


extern usbd_desc_handler printer_desc_handler;

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
