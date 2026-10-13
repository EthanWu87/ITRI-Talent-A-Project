/*********************************************************************************************************//**
 * @file    audio_class.h
 * @version $Rev:: 180         $
 * @date    $Date:: 2025-12-09 #$
 * @brief   usb audio class file
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
#ifndef __AUDIO_CLASS_H
#define __AUDIO_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_std.h"
#include "usbd_core.h"
#include "audio_conf.h"

/** @addtogroup HT32F493x5_middlewares_usbd_class
  * @{
  */

/** @addtogroup USB_audio_class
  * @{
  */

/** @defgroup USB_audio_class_definition
  * @{
  */

/**
  * @brief endpoint define
  */
#define USBD_AUDIO_MIC_IN_EPT            0x81
#define USBD_AUDIO_SPK_OUT_EPT           0x02
#define USBD_AUDIO_FEEDBACK_EPT          0x83

/**
  * @brief endpoint support max size
  */
#define AUDIO_REMAIN_SIZE                40
#define AUDIO_MIC_IN_MAXPACKET_SIZE      (AUDIO_SUPPORT_MAX_FREQ * AUDIO_MIC_CHANEL_NUM * (AUDIO_MIC_DEFAULT_BITW / 8) + AUDIO_REMAIN_SIZE)
#define AUDIO_SPK_OUT_MAXPACKET_SIZE     (AUDIO_SUPPORT_MAX_FREQ * AUDIO_SPK_CHANEL_NUM * (AUDIO_SPK_DEFAULT_BITW / 8) + AUDIO_REMAIN_SIZE)
#define AUDIO_FEEDBACK_MAXPACKET_SIZE    0x3
#define FEEDBACK_REFRESH_TIME            0x8
/**
  * @brief request type define
  */
#define AUDIO_REQ_CONTROL_INTERFACE      0x01
#define AUDIO_REQ_CONTROL_ENDPOINT       0x02
#define AUDIO_REQ_CONTROL_MASK           0x03

/**
  * @brief audio set cur type define
  */
#define AUDIO_MUTE_CONTROL               0x01
#define AUDIO_VOLUME_CONTROL             0x02
#define AUDIO_FREQ_SET_CONTROL           0x03
#define AUDIO_SAMPLING_FREQ_CONTROL      0x01

/**
  * @brief audio descriptor type
  */
#define AUDIO_DESCRIPTOR_TYPE             0x21
#define AUDIO_DESCRIPTOR_SIZE             0x09

/**
  * @brief usb audio control struct
  */
typedef struct
{
  uint8_t enpd;
  uint8_t interface;
  uint8_t request_no;
  uint8_t spk_mute;
  uint8_t mic_mute;
  uint16_t spk_volume;
  uint16_t mic_volume;
  uint32_t spk_freq;
  uint32_t mic_freq;
  uint16_t spk_volume_limits[3]; /*[0] is mininum value, [1] is maxnum value, [2] is volume resolution */
  uint16_t mic_volume_limits[3]; /*[0] is mininum value, [1] is maxnum value, [2] is volume resolution */

  uint8_t audio_cmd;
  uint32_t audio_cmd_len;
  uint32_t spk_alt_setting;
  uint32_t mic_alt_setting;
  uint8_t g_audio_cur[64];
  uint8_t audio_spk_data[AUDIO_SPK_OUT_MAXPACKET_SIZE];
  uint8_t audio_mic_data[AUDIO_MIC_IN_MAXPACKET_SIZE];
  uint8_t audio_feed_back[AUDIO_FEEDBACK_MAXPACKET_SIZE+1];
}usb_audio_type;

extern usbd_class_handler audio_class_handler;

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

