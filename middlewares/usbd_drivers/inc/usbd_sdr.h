/*********************************************************************************************************//**
 * @file    usbd_sdr.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb standard request header file
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
#ifndef __USBD_SDR_H
#define __USBD_SDR_H

#ifdef __cplusplus
extern "C" {
#endif


/* includes ------------------------------------------------------------------*/
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_drivers
  * @{
  */

/** @addtogroup USBD_drivers_standard_request
  * @{
  */

/** @defgroup USBD_sdr_exported_functions
  * @{
  */

void usbd_setup_request_parse(usb_setup_type *setup, uint8_t *buf);
usb_sts_type usbd_device_request(usbd_core_type *udev);
usb_sts_type usbd_interface_request(usbd_core_type *udev);
usb_sts_type usbd_endpoint_request(usbd_core_type *udev);


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

