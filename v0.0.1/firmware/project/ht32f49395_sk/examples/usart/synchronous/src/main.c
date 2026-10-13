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

/** @addtogroup HT32F493x5_USART_synchronous USART_synchronous
  * @{
  */

#define COUNTOF(a)                       (sizeof(a) / sizeof(*(a)))
#define USART2_TX_BUFFER_SIZE            (COUNTOF(usart2_tx_buffer) - 1)
#define SPI2_TX_BUFFER_SIZE              (COUNTOF(spi2_tx_buffer) - 1)
#define DYMMY_BYTE                       0x00

uint8_t usart2_tx_buffer[] = "usart synchronous example: usart2 -> spi2";
uint8_t spi2_tx_buffer[] = "usart synchronous example: spi2 -> usart2";
uint8_t usart2_rx_buffer[SPI2_TX_BUFFER_SIZE];
uint8_t spi2_rx_buffer[USART2_TX_BUFFER_SIZE];
uint8_t usart2_tx_counter = 0x00;
uint8_t spi2_tx_counter = 0x00;
uint8_t usart2_rx_counter = 0x00;
uint8_t spi2_rx_counter = 0x00;
uint8_t usart2_tx_buffer_size = USART2_TX_BUFFER_SIZE;
uint8_t spi2_tx_buffer_size = SPI2_TX_BUFFER_SIZE;

/**
  * @brief  config usart and spi
  * @param  none
  * @retval none
  */
void usart_spi_configuration(void)
{
  gpio_init_type gpio_init_struct;
  spi_init_type spi_init_struct;

  /* enable the usart2 and gpio clock */
  crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  /* enable the spi2 and gpio clock */
  crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the usart2 tx and ck pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_2 | GPIO_PINS_4;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  /* configure the usart2 rx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_3;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOA, &gpio_init_struct);

  /* configure the spi2 mosi and ck pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_15 | GPIO_PINS_13;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(GPIOB, &gpio_init_struct);

  /* configure the spi2 miso pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_14;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOB, &gpio_init_struct);

  /* configure usart2 param */
  usart_init(USART2, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_clock_config(USART2, USART_CLOCK_POLARITY_HIGH, USART_CLOCK_PHASE_2EDGE, USART_CLOCK_LAST_BIT_OUTPUT);
  usart_clock_enable(USART2, TRUE);
  usart_transmitter_enable(USART2, TRUE);
  usart_receiver_enable(USART2, TRUE);
  usart_enable(USART2, TRUE);

  /* configure spi2 param */
  spi_default_para_init(&spi_init_struct);
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode =SPI_MODE_SLAVE;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_LSB;
  spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI2, &spi_init_struct);
  spi_enable(SPI2, TRUE);
}

/**
  * @brief  compares two buffers.
  * @param  pbuffer1, pbuffer2: buffers to be compared.
  * @param  buffer_length: buffer's length
  * @retval 1: pbuffer1 identical to pbuffer2
  *         0: pbuffer1 differs from pbuffer2
  */
uint8_t buffer_compare(uint8_t* pbuffer1, uint8_t* pbuffer2, uint16_t buffer_length)
{
  while(buffer_length--)
  {
    if(*pbuffer1 != *pbuffer2)
    {
      return 0;
    }
    pbuffer1++;
    pbuffer2++;
  }
  return 1;
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
  usart_spi_configuration();

  /* wait until end of transmission from usart2 to spi2 */
  while(spi2_rx_counter < usart2_tx_buffer_size)
  {
    /* usart transmit data */
    while(usart_flag_get(USART2, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(USART2, usart2_tx_buffer[usart2_tx_counter++]);

    /* wait until spi receive data */
    while(spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET);
    spi2_rx_buffer[spi2_rx_counter++] = spi_i2s_data_receive(SPI2);

    /* wait until usart receive data, and clear dummy byte in data register */
    while(usart_flag_get(USART2, USART_RDBF_FLAG) == RESET);
    usart_data_receive(USART2);
  }

  /* wait until end of transmission from spi2 to usart2 */
  while(usart2_rx_counter < spi2_tx_buffer_size)
  {
    /* spi put transmit data in data register */
    while(spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI2, spi2_tx_buffer[spi2_tx_counter++]);

    /* usart transmit a dummy byte to generate clock to slave */
    while(usart_flag_get(USART2, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(USART2, DYMMY_BYTE);

    /* wait until usart receive data */
    while(usart_flag_get(USART2, USART_RDBF_FLAG) == RESET);
    usart2_rx_buffer[usart2_rx_counter++] = usart_data_receive(USART2);
  }

  while(1)
  {
    /* compare data buffer */
    if(buffer_compare(usart2_tx_buffer, spi2_rx_buffer, USART2_TX_BUFFER_SIZE) && \
       buffer_compare(spi2_tx_buffer, usart2_rx_buffer, SPI2_TX_BUFFER_SIZE))
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
