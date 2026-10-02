/*********************************************************************************************************//**
 * @file    usb_int.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb header file
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
#ifndef __USB_INT_H
#define __USB_INT_H

#ifdef __cplusplus
extern "C" {
#endif


/* includes ------------------------------------------------------------------*/
#include "usbd_core.h"

/** @addtogroup HT32F493x5_middlewares_usbd_drivers
  * @{
  */

/** @addtogroup USBD_drivers_interrupt
  * @{
  */

/** @defgroup USBD_int_exported_functions
  * @{
  */
void usbd_irq_handler(usbd_core_type *udev);
void usbd_ept_loop_handler(usbd_core_type *udev);
void usbd_eptn_handler(usbd_core_type *udev, usb_ept_number_type ept_num);
void usbd_reset_handler(usbd_core_type *udev);
void usbd_sof_handler(usbd_core_type *udev);
void usbd_suspend_handler(usbd_core_type *udev);
void usbd_wakeup_handler(usbd_core_type *udev);

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

