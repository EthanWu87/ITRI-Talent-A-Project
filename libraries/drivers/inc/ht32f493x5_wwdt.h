/*********************************************************************************************************//**
 * @file    ht32f493x5_wwdt.h
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   ht32f493x5 wwdt header file
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
#ifndef __HT32F493x5_WWDT_H
#define __HT32F493x5_WWDT_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "ht32f493x5.h"

/** @addtogroup HT32F493x5_periph_driver
  * @{
  */

/** @addtogroup WWDT
  * @{
  */

/** @defgroup WWDT_enable_bit_definition
  * @brief wwdt enable bit
  * @{
  */

#define WWDT_EN_BIT                      ((uint32_t)0x00000080) /*!< wwdt enable bit */

/**
  * @}
  */

/** @defgroup WWDT_exported_types
  * @{
  */

/**
  * @brief wwdt division type
  */
typedef enum
{
  WWDT_PCLK1_DIV_4096                    = 0x00, /*!< wwdt counter clock = (pclk1/4096)/1) */
  WWDT_PCLK1_DIV_8192                    = 0x01, /*!< wwdt counter clock = (pclk1/4096)/2) */
  WWDT_PCLK1_DIV_16384                   = 0x02, /*!< wwdt counter clock = (pclk1/4096)/4) */
  WWDT_PCLK1_DIV_32768                   = 0x03  /*!< wwdt counter clock = (pclk1/4096)/8) */
} wwdt_division_type;

/**
  * @brief type define wwdt register all
  */
typedef struct
{

  /**
    * @brief wwdt ctrl register, offset:0x00
    */
  union
  {
    __IO uint32_t ctrl;
    struct
    {
      __IO uint32_t cnt                  : 7; /* [6:0] */
      __IO uint32_t wwdten               : 1; /* [7] */
      __IO uint32_t reserved1            : 24;/* [31:8] */
    } ctrl_bit;
  };

  /**
    * @brief wwdt cfg register, offset:0x04
    */
  union
  {
    __IO uint32_t cfg;
    struct
    {
      __IO uint32_t win                  : 7; /* [6:0] */
      __IO uint32_t div                  : 2; /* [8:7] */
      __IO uint32_t rldien               : 1; /* [9] */
      __IO uint32_t reserved1            : 22;/* [31:10] */
    } cfg_bit;
  };

  /**
    * @brief wwdt cfg register, offset:0x08
    */
  union
  {
    __IO uint32_t sts;
    struct
    {
      __IO uint32_t rldf                 : 1; /* [0] */
      __IO uint32_t reserved1            : 31;/* [31:1] */
    } sts_bit;
  };

} wwdt_type;

/**
  * @}
  */

#define WWDT                             ((wwdt_type *) WWDT_BASE)

/** @defgroup WWDT_exported_functions
  * @{
  */

void wwdt_reset(void);
void wwdt_divider_set(wwdt_division_type division);
void wwdt_flag_clear(void);
void wwdt_enable(uint8_t wwdt_cnt);
void wwdt_interrupt_enable(void);
flag_status wwdt_flag_get(void);
flag_status wwdt_interrupt_flag_get(void);
void wwdt_counter_set(uint8_t wwdt_cnt);
void wwdt_window_counter_set(uint8_t window_cnt);

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
