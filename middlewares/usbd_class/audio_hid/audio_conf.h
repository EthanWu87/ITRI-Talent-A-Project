/*********************************************************************************************************//**
 * @file    audio_conf.h
 * @version $Rev:: 131         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   usb audio config
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
#ifndef __AUDIO_CONF_H
#define __AUDIO_CONF_H

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_audio_hid_class
  * @{
  */

/** @defgroup USB_device_audio_hid_config_definition
  * @{
  */
#define AUDIO_SUPPORT_SPK                1
#define AUDIO_SUPPORT_MIC                1
#define AUDIO_SUPPORT_FEEDBACK           1

#define AUDIO_SUPPORT_FREQ_16K           0
#define AUDIO_SUPPORT_FREQ_48K           1


#define AUDIO_SUPPORT_FREQ               (AUDIO_SUPPORT_FREQ_16K + \
                                          AUDIO_SUPPORT_FREQ_48K \
                                         )

#define AUDIO_FREQ_16K                   16000
#define AUDIO_FREQ_48K                   48000
#define AUDIO_BITW_16                    16

#define AUDIO_MIC_CHANEL_NUM            2
#define AUDIO_MIC_DEFAULT_BITW          AUDIO_BITW_16

#define AUDIO_SPK_CHANEL_NUM            2
#define AUDIO_SPK_DEFAULT_BITW          AUDIO_BITW_16


#define AUDIO_SUPPORT_MAX_FREQ           48
#define AUDIO_DEFAULT_FREQ               AUDIO_FREQ_48K
#define AUDIO_DEFAULT_BITW               AUDIO_BITW_16

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



