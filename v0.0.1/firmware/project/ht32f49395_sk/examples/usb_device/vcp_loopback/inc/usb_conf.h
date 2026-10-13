/*********************************************************************************************************//**
 * @file    usb_conf.h
 * @version $Rev:: 135         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb config header file
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
#ifndef __USB_CONF_H
#define __USB_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ht32f493x5.h"
#include "ht32f493x5_board.h"
#include "stdio.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_USB_device_vcp_loopback
  * @{
  */

/**
  * @brief usb endpoint max num define
  */
#ifndef USB_EPT_MAX_NUM
#define USB_EPT_MAX_NUM                   8  /*!< usb device support endpoint number */
#endif

/**
  * @brief usb buffer extend to 768-1280 bytes
  */
//#define USB_BUFFER_SIZE_EX  /*!< usb enable extend buffer */


/**
  * @brief auto malloc usb endpoint buffer
  */
//#define USB_EPT_AUTO_MALLOC_BUFFER  /*!< usb auto malloc endpoint tx and rx buffer */


#ifndef USB_EPT_AUTO_MALLOC_BUFFER
/**
  * @brief user custom endpoint buffer
  *        EPTn_TX_ADDR, EPTn_RX_ADDR must less than usb buffer size
  */

/* ept0 tx start address 0x40, size 0x40,
   so rx start address is 0x40 + 0x40 = 0x80 */
#define EPT0_TX_ADDR                     0x40    /*!< usb endpoint 0 tx buffer address offset */
#define EPT0_RX_ADDR                     0x80    /*!< usb endpoint 0 rx buffer address offset */

#define EPT1_TX_ADDR                     0xC0    /*!< usb endpoint 1 tx buffer address offset */
#define EPT1_RX_ADDR                     0x100   /*!< usb endpoint 1 rx buffer address offset */

#define EPT2_TX_ADDR                     0x140   /*!< usb endpoint 2 tx buffer address offset */
#define EPT2_RX_ADDR                     0x180   /*!< usb endpoint 2 rx buffer address offset */

#define EPT3_TX_ADDR                     0x00    /*!< usb endpoint 3 tx buffer address offset */
#define EPT3_RX_ADDR                     0x00    /*!< usb endpoint 3 rx buffer address offset */

#define EPT4_TX_ADDR                     0x00    /*!< usb endpoint 4 tx buffer address offset */
#define EPT4_RX_ADDR                     0x00    /*!< usb endpoint 4 rx buffer address offset */

#define EPT5_TX_ADDR                     0x00    /*!< usb endpoint 5 tx buffer address offset */
#define EPT5_RX_ADDR                     0x00    /*!< usb endpoint 5 rx buffer address offset */

#define EPT6_TX_ADDR                     0x00    /*!< usb endpoint 6 tx buffer address offset */
#define EPT6_RX_ADDR                     0x00    /*!< usb endpoint 6 rx buffer address offset */

#define EPT7_TX_ADDR                     0x00    /*!< usb endpoint 7 tx buffer address offset */
#define EPT7_RX_ADDR                     0x00    /*!< usb endpoint 7 rx buffer address offset */

#endif

void usb_delay_ms(uint32_t ms);
void usb_delay_us(uint32_t us);

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

