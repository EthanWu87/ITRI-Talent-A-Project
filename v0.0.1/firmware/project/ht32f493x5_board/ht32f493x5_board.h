/*********************************************************************************************************//**
 * @file    ht32f493x5_board.h
 * @version $Rev:: 162         $
 * @date    $Date:: 2025-09-03 #$
 * @brief   header file for ht32f49395_sk board.
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

#ifndef __HT32F493x5_BOARD_H
#define __HT32F493x5_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "ht32f493x5.h"

/** @addtogroup HT32F493x5_board
  * @{
  */

/** @addtogroup BOARD
  * @{
  */

/** @defgroup BOARD_pins_definition
  * @{
  */

/**
  * this header include define support list:
  * 1. ht32f49395_sk v1.x board
  * if define USE_HT32F49395_SK, the header file support ht32f49395_sk v1.x board
  */

#if !defined (USE_HT32F49395_SK)
#error "please select first the board ht32f49395_sk device used in your application (in ht32f493x5_board.h file)"
#endif

/******************** define led ********************/
typedef enum
{
  LED3                                   = 0
} led_type;

#define LED_NUM                          1

#if defined (USE_HT32F49395_SK)

#define LED3_PIN                         GPIO_PINS_11
#define LED3_GPIO                        GPIOE
#define LED3_GPIO_CRM_CLK                CRM_GPIOE_PERIPH_CLOCK

#endif

/**************** define print uart ******************/
#define PRINT_UART                       USART1
#define PRINT_UART_CRM_CLK               CRM_USART1_PERIPH_CLOCK
#define PRINT_UART_TX_PIN                GPIO_PINS_9
#define PRINT_UART_TX_GPIO               GPIOA
#define PRINT_UART_TX_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK

#define PRINT_UART_RX_PIN                GPIO_PINS_10
#define PRINT_UART_RX_GPIO               GPIOA
#define PRINT_UART_RX_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK

/******************* define button *******************/
typedef enum
{
  USER_BUTTON                            = 0,
  NO_BUTTON                              = 1
} button_type;

#define USER_BUTTON_PIN                  GPIO_PINS_0
#define USER_BUTTON_PORT                 GPIOA
#define USER_BUTTON_CRM_CLK              CRM_GPIOA_PERIPH_CLOCK

/**
  * @}
  */

/** @defgroup BOARD_exported_functions
  * @{
  */

/******************** functions ********************/
void ht32_board_init(void);

/* led operation function */
void ht32_led_init(led_type led);
void ht32_led_on(led_type led);
void ht32_led_off(led_type led);
void ht32_led_toggle(led_type led);

/* button operation function */
void ht32_button_init(void);
button_type ht32_button_press(void);
uint8_t ht32_button_state(void);

/* delay function */
void delay_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
void delay_sec(uint16_t sec);

/* printf uart init function */
void uart_print_init(uint32_t baudrate);

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

