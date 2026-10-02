/*********************************************************************************************************//**
 * @file    main.c
 * @version $Rev:: 142         $
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

/** @addtogroup HT32F493x5_GPIO_swjtag_remap GPIO_swjtag_remap
  * @{
  */

void swj_dp_config(void);
void gpio_pins_toggle(gpio_type* gpio_x, uint16_t gpio_pin);

/**
  * @brief  configure the gpio of swj-dp.
  * @param  none
  * @retval none
  */
void swj_dp_config(void)
{
  gpio_init_type gpio_init_struct;

  /* configure pa13 (jtms/swdat), pa14 (jtck/swclk) and pa15 (jtdi) as output push-pull */
  gpio_init_struct.gpio_pins = GPIO_PINS_13 | GPIO_PINS_14 | GPIO_PINS_15;
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init(GPIOA, &gpio_init_struct);

  /* configure pb3 (jtdo) and pb4 (jtrst) as output push-pull */
  gpio_init_struct.gpio_pins = GPIO_PINS_3 | GPIO_PINS_4;
  gpio_init(GPIOB, &gpio_init_struct);
}

/**
  * @brief  toggles the specified gpio pin
  * @param  gpiox: where x can be (a..g depending on device used) to select the gpio peripheral
  * @param  gpio_pin: specifies the pins to be toggled.
  * @retval none
  */
void gpio_pins_toggle(gpio_type* gpio_x, uint16_t gpio_pin)
{
  gpio_x->odt ^= gpio_pin;
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

  crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  swj_dp_config();

  while(1)
  {
    if(USER_BUTTON == ht32_button_press())
    {
      /* disable the serial wire jtag debug port swj-dp */
      gpio_pin_remap_config(SWJTAG_MUX_100, TRUE);

      /* turn on led3 */
      ht32_led_on(LED3);
    }

    /* toggle jtms/swdat pin */
    gpio_pins_toggle(GPIOA, GPIO_PINS_13);
    delay_us(200);

    /* toggle jtck/swclk pin */
    gpio_pins_toggle(GPIOA, GPIO_PINS_14);
    delay_us(200);

    /* toggle jtdi pin */
    gpio_pins_toggle(GPIOA, GPIO_PINS_15);
    delay_us(200);

    /* toggle jtdo pin */
    gpio_pins_toggle(GPIOB, GPIO_PINS_3);
    delay_us(200);

    /* toggle jtrst pin */
    gpio_pins_toggle(GPIOB, GPIO_PINS_4);
    delay_us(200);
  }
}

/**
  * @}
  */

/**
  * @}
  */
