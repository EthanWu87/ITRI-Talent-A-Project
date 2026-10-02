/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 138         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   main program
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

#include "ht32f493x5_board.h"
#include "ht32f493x5_clock.h"

/** @addtogroup HT32F493x5_periph_examples
  * @{
  */

/** @addtogroup HT32F493x5_USART_receiver_mute USART_receiver_mute
  * @{
  */

#define BUFFER_SIZE                      7
#define MATCH_ID_VAL                     0x81
#define ERROR_ID_VAL                     0x82
#define DATA1_VAL                        0x7F
#define DATA2_VAL                        0x7A
#define DATA3_VAL                        0x7B
#define DATA4_VAL                        0x7C

uint8_t usart2_tx_buffer[BUFFER_SIZE] = {DATA1_VAL, MATCH_ID_VAL, DATA2_VAL, ERROR_ID_VAL, DATA3_VAL, MATCH_ID_VAL, DATA4_VAL};
uint8_t usart3_rx_buffer[BUFFER_SIZE];
uint8_t tx_counter = 0;
uint8_t rx_counter = 0;

/**
  * @brief  config usart
  * @param  none
  * @retval none
  */
void usart_configuration(void)
{
  gpio_init_type gpio_init_struct;

  /* enable the usart2 and gpio clock */
  crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  /* enable the usart3 and gpio clock */
  crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the usart2 tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_2;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  /* configure the usart3 rx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_11;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOB, &gpio_init_struct);

  /* configure usart2 param */
  usart_init(USART2, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(USART2, TRUE);
  usart_enable(USART2, TRUE);

  /* config usart3 nvic interrupt */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(USART3_IRQn, 0 ,0);

  /* configure usart3 param */
  usart_init(USART3, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_receiver_enable(USART3, TRUE);
  usart_interrupt_enable(USART3, USART_RDBF_INT, TRUE);
  usart_wakeup_id_set(USART3, 0x01);
  usart_wakeup_mode_set(USART3, USART_WAKEUP_BY_MATCHING_ID);
  usart_receiver_mute_enable(USART3, TRUE);
  usart_enable(USART3, TRUE);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();
  ht32_board_init();
  usart_configuration();

  /* usart2 transmit data */
  while(tx_counter < BUFFER_SIZE)
  {
    while(usart_flag_get(USART2, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(USART2, usart2_tx_buffer[tx_counter++]);
  }

  /* wait usart2 data transmission complete */
  while(usart_flag_get(USART2, USART_TDC_FLAG) == RESET);

  while(1)
  {
    if((usart3_rx_buffer[0] == MATCH_ID_VAL) && (usart3_rx_buffer[1] == DATA2_VAL) && \
      (usart3_rx_buffer[2] == MATCH_ID_VAL) && (usart3_rx_buffer[3] == DATA4_VAL))
    {
      ht32_led_toggle(LED2);
      ht32_led_toggle(LED3);
      ht32_led_toggle(LED4);
      delay_sec(1);
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */
