/*********************************************************************************************************//**
 * @file    smartcard_config.h
 * @version $Rev:: 138         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   header file of clock program
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
#ifndef __SMARTCARD_CONFIG_H
#define __SMARTCARD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* smartcard inteface usart pins */
#define SC_USART                 USART2
#define SC_USART_CLK             CRM_USART2_PERIPH_CLOCK
#define SC_USART_GPIO_CLK        CRM_GPIOA_PERIPH_CLOCK
#define SC_USART_TX_PIN          GPIO_PINS_2
#define SC_USART_TX_PORT         GPIOA
#define SC_USART_CLK_PIN         GPIO_PINS_4
#define SC_USART_CLK_PORT        GPIOA
#define SC_USART_IRQn            USART2_IRQn
#define SC_USART_IRQHandler      USART2_IRQHandler

/* smartcard inteface gpio pins */
#define SC_3_5V_PIN              GPIO_PINS_5
#define SC_RESET_PIN             GPIO_PINS_6
#define SC_CMDVCC_PIN            GPIO_PINS_7
#define SC_OFF_PIN               GPIO_PINS_8
#define SC_3_5V_PORT             GPIOA
#define SC_RESET_PORT            GPIOA
#define SC_CMDVCC_PORT           GPIOA
#define SC_OFF_PORT              GPIOA
#define SC_3_5V_CLK              CRM_GPIOA_PERIPH_CLOCK
#define SC_RESET_CLK             CRM_GPIOA_PERIPH_CLOCK
#define SC_CMDVCC_CLK            CRM_GPIOA_PERIPH_CLOCK
#define SC_OFF_CLK               CRM_GPIOA_PERIPH_CLOCK
#define SC_OFF_EXINT             EXINT_LINE_8
#define SC_OFF_PORTSOURCE        GPIO_PORT_SOURCE_GPIOA
#define SC_OFF_PINSOURCE         GPIO_PINS_SOURCE8
#define SC_OFF_EXINT_IRQ         EXINT9_5_IRQn

#ifdef __cplusplus
}
#endif

#endif /* __SMARTCARD_CONFIG_H */

