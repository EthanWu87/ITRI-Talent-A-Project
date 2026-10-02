/*********************************************************************************************************//**
 * @file    touch.h
 * @version $Rev:: 140         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   this file contains all the functions prototypes for the
 *          touch firmware driver.
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

#ifndef __TOUCH_H
#define __TOUCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ht32f493x5_board.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_XMC_lcd_touch_16bit
  * @{
  */

/** @defgroup TOUCH_device_type
  * @{
  */

typedef struct
{
  void (*init)(void);                                   /*!< pin init */
  void (*touch_read_xy)(uint16_t *x, uint16_t *y);      /*!<  read x/y coordinate */
  void (*touch_scan)(void);                             /*!<  touch scan func */
  error_status (*touch_adjust)(void);                   /*!<  touch adjust func */
  uint16_t x_p[6];                                      /*!<  save x coordinate */
  uint16_t y_p[6];                                      /*!<  save y coordinate */
}touch_dev_type;

/**
  * @}
  */

#define TCLK_HIGH                        gpio_bits_set(GPIOB,GPIO_PINS_3)
#define TCLK_LOW                         gpio_bits_reset(GPIOB,GPIO_PINS_3)

#define TDOUT                            gpio_input_data_bit_read(GPIOB,GPIO_PINS_4)

#define TDIN_HIGH                        gpio_bits_set(GPIOB,GPIO_PINS_5)
#define TDIN_LOW                         gpio_bits_reset(GPIOB,GPIO_PINS_5)

#define PEN_HIGH                         gpio_input_data_bit_read(GPIOD,GPIO_PINS_5)

#define TCS_HIGH                         gpio_bits_set(GPIOB,GPIO_PINS_8)
#define TCS_LOW                          gpio_bits_reset(GPIOB,GPIO_PINS_8)

extern touch_dev_type touch_dev_struct;
extern touch_dev_type *touch_struct;

/** @defgroup TOUCH_exported_functions
  * @{
  */

void touch_pin_init(void);
void touch_read_xy(uint16_t *x, uint16_t *y);
void touch_scan(void);
uint16_t touch_read_x_or_y(uint8_t xy);
uint16_t touch_read_ad(uint8_t xy);
void touch_write_1byte(uint8_t cmd);
uint8_t touch_read_1byte(void);
error_status touch_adjust(void);

/**
  * @}
  */

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
