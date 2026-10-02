/*********************************************************************************************************//**
 * @file    ht32f493x5_acc.h
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   ht32f493x5 acc header file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HT32F493x5_ACC_H
#define __HT32F493x5_ACC_H


#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "ht32f493x5.h"

/** @addtogroup HT32F493x5_periph_driver
  * @{
  */

/** @addtogroup ACC
  * @{
  */

/** @defgroup ACC_exported_constants
  * @{
  */

#define ACC_CAL_HICKCAL                  ((uint16_t)0x0000) /*!< acc hick calibration */
#define ACC_CAL_HICKTRIM                 ((uint16_t)0x0002) /*!< acc hick trim */

#define ACC_RSLOST_FLAG                  ((uint16_t)0x0002) /*!< acc reference signal lost error flag */
#define ACC_CALRDY_FLAG                  ((uint16_t)0x0001) /*!< acc internal high-speed clock calibration ready error flag */

#define ACC_CALRDYIEN_INT                ((uint16_t)0x0020) /*!< acc internal high-speed clock calibration ready interrupt enable */
#define ACC_EIEN_INT                     ((uint16_t)0x0010) /*!< acc reference signal lost interrupt enable */

/**
  * @}
  */

/** @defgroup ACC_exported_types
  * @{
  */

/**
  * @brief type define acc register all
  */
typedef struct
{

  /**
    * @brief acc sts register, offset:0x00
    */
  union
  {
    __IO uint32_t sts;
    struct
    {
      __IO uint32_t calrdy               : 1; /* [0] */
      __IO uint32_t rslost               : 1; /* [1] */
      __IO uint32_t reserved1            : 30;/* [31:2] */
    } sts_bit;
  };

  /**
    * @brief acc ctrl1 register, offset:0x04
    */
  union
  {
    __IO uint32_t ctrl1;
    struct
    {
      __IO uint32_t calon                : 1; /* [0] */
      __IO uint32_t entrim               : 1; /* [1] */
      __IO uint32_t reserved1            : 2; /* [3:2] */
      __IO uint32_t eien                 : 1; /* [4] */
      __IO uint32_t calrdyien            : 1; /* [5] */
      __IO uint32_t reserved2            : 2; /* [7:6] */
      __IO uint32_t step                 : 4; /* [11:8] */
      __IO uint32_t reserved3            : 20;/* [31:12] */
    } ctrl1_bit;
  };

   /**
    * @brief acc ctrl2 register, offset:0x08
    */
  union
  {
    __IO uint32_t ctrl2;
    struct
    {
      __IO uint32_t hickcal              : 8; /* [7:0] */
      __IO uint32_t hicktrim             : 6; /* [13:8] */
      __IO uint32_t reserved1            : 18;/* [31:14] */
    } ctrl2_bit;
  };

  /**
  * @brief acc acc_c1 register, offset:0x0C
  */
  union
  {
    __IO uint32_t c1;
    struct
    {
      __IO uint32_t c1                   : 16;/* [15:0] */
      __IO uint32_t reserved1            : 16;/* [31:16] */
    } c1_bit;
  };

  /**
  * @brief acc acc_c2 register, offset:0x10
  */
  union
  {
    __IO uint32_t c2;
    struct
    {
      __IO uint32_t c2                   : 16;/* [15:0] */
      __IO uint32_t reserved1            : 16;/* [31:16] */
    } c2_bit;
  };

  /**
  * @brief acc acc_c3 register, offset:0x14
  */
  union
  {
    __IO uint32_t c3;
    struct
    {
      __IO uint32_t c3                   : 16;/* [15:0] */
      __IO uint32_t reserved1            : 16;/* [31:16] */
    } c3_bit;
  };
} acc_type;

/**
  * @}
  */

#define ACC                             ((acc_type *) ACC_BASE)

/** @defgroup ACC_exported_functions
  * @{
  */

void acc_calibration_mode_enable(uint16_t acc_trim, confirm_state new_state);
void acc_step_set(uint8_t step_value);
void acc_interrupt_enable(uint16_t acc_int, confirm_state new_state);
uint8_t acc_hicktrim_get(void);
uint8_t acc_hickcal_get(void);
void acc_write_c1(uint16_t acc_c1_value);
void acc_write_c2(uint16_t acc_c2_value);
void acc_write_c3(uint16_t acc_c3_value);
uint16_t acc_read_c1(void);
uint16_t acc_read_c2(void);
uint16_t acc_read_c3(void);
flag_status acc_flag_get(uint16_t acc_flag);
flag_status acc_interrupt_flag_get(uint16_t acc_flag);
void acc_flag_clear(uint16_t acc_flag);

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
