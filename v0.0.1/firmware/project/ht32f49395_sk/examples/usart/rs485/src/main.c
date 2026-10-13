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

/** @addtogroup HT32F493x5_USART_rs485
  * @{
  */

#define RS485_BAUDRATE                       9600
#define RS485_BUFFER_SIZE                    128

uint8_t rs485_buffer_rx[RS485_BUFFER_SIZE];
uint8_t rs485_buffer_rx_cnt = 0;

/**
  *  @brief  rs485 configiguration.
  *  @param  none
  *  @retval none
  */
static void rs485_config(void)
{
  gpio_init_type gpio_init_struct;

  /* enable the uart2 and gpio clock */
  crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the uart2 tx,rx,de pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_2;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_init_struct.gpio_pins = GPIO_PINS_3;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_init_struct.gpio_pins = GPIO_PINS_1;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_bits_reset(GPIOA, GPIO_PINS_1);

  /* configure uart2 param */
  usart_init(USART2, RS485_BAUDRATE, USART_DATA_8BITS, USART_STOP_1_BIT);

  usart_flag_clear(USART2, USART_RDBF_FLAG);
  usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);

  usart_receiver_enable(USART2, TRUE);
  usart_transmitter_enable(USART2, TRUE);
  usart_enable(USART2, TRUE);

  nvic_irq_enable(USART2_IRQn, 1, 0);
}

/**
  *  @brief  rs485 send data
  *  @param  buf: pointer to the buffer that contain the data to be transferred.
  *  @param  cnt: size of buffer in bytes.
  *  @retval none
  */
static void rs485_send_data(u8* buf, u8 cnt)
{
  gpio_bits_set(GPIOA, GPIO_PINS_1);
  while(cnt--){
    while(usart_flag_get(USART2, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(USART2, *buf++);
  }
  while(usart_flag_get(USART2, USART_TDC_FLAG) == RESET);
  gpio_bits_reset(GPIOA, GPIO_PINS_1);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  char str[]="start test..\r\n";
  u8 len = 0;

  system_clock_config();
  ht32_board_init();
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  rs485_config();

  len = sizeof(str);
  rs485_send_data((u8*)str, len);
  while(1)
  {
    if(usart_flag_get(USART2, USART_IDLEF_FLAG) != RESET)
    {
      usart_data_receive(USART2);
      usart_interrupt_enable(USART2, USART_RDBF_INT, FALSE);
      rs485_send_data(rs485_buffer_rx, rs485_buffer_rx_cnt);
      rs485_buffer_rx_cnt = 0;
      usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);
    }
  }
}

/**
  * @brief  usart2 interrupt handler
  * @param  none
  * @retval none
  */
void USART2_IRQHandler(void)
{
  uint16_t tmp;

  if(usart_interrupt_flag_get(USART2, USART_RDBF_FLAG) != RESET)
  {
    tmp = usart_data_receive(USART2);
    if(rs485_buffer_rx_cnt < RS485_BUFFER_SIZE)
    {
      rs485_buffer_rx[rs485_buffer_rx_cnt++] = tmp;
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */
