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

/** @addtogroup HT32F493x5_BPR_data BPR_data
  * @{
  */

#define BPR_DR_NUMBER                    42

bpr_data_type bpr_addr_tab[BPR_DR_NUMBER] =
{
  BPR_DATA1,  BPR_DATA2,  BPR_DATA3,  BPR_DATA4,  BPR_DATA5,
  BPR_DATA6,  BPR_DATA7,  BPR_DATA8,  BPR_DATA9,  BPR_DATA10,
  BPR_DATA11, BPR_DATA12, BPR_DATA13, BPR_DATA14, BPR_DATA15,
  BPR_DATA16, BPR_DATA17, BPR_DATA18, BPR_DATA19, BPR_DATA20,
  BPR_DATA21, BPR_DATA22, BPR_DATA23, BPR_DATA24, BPR_DATA25,
  BPR_DATA26, BPR_DATA27, BPR_DATA28, BPR_DATA29, BPR_DATA30,
  BPR_DATA31, BPR_DATA32, BPR_DATA33, BPR_DATA34, BPR_DATA35,
  BPR_DATA36, BPR_DATA37, BPR_DATA38, BPR_DATA39, BPR_DATA40,
  BPR_DATA41, BPR_DATA42
};

void bpr_reg_write(void);
uint8_t bpr_reg_check(void);
void uart_init(uint32_t baudrate);

/**
  * @brief  write data to bpr dt register.
  * @param  none.
  * @retval none
  */
void bpr_reg_write(void)
{
  uint32_t index = 0;

  for(index = 0; index < BPR_DR_NUMBER; index++)
  {
    bpr_data_write(bpr_addr_tab[index], 0x5A00 | bpr_addr_tab[index]);
  }
}

/**
  * @brief  checks if the bpr dt register values are correct or not.
  * @param  none.
  * @retval
  *          - TRUE: all bpr dt register values are correct
  *          - FALSE: number of the bpr register
  *            which value is not correct
  */
uint8_t bpr_reg_check(void)
{
  uint32_t index = 0;

  for(index = 0; index < BPR_DR_NUMBER; index++)
  {
    if(bpr_data_read(bpr_addr_tab[index]) != (0x5A00 | bpr_addr_tab[index]))
    {
      return FALSE;
    }
  }

  return TRUE;
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

  uart_print_init(115200);

  /* enable pwc and bpr clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_BPR_PERIPH_CLOCK, TRUE);

  /* enable write access to bpr domain */
  pwc_battery_powered_domain_access(TRUE);

  /* clear tamper pin event pending flag */
  bpr_flag_clear(BPR_TAMPER_EVENT_FLAG);

  /* check data from bpr dt register */
  if(bpr_reg_check() == TRUE)
  {
    printf("bpr reg => none reset\r\n");
  }
  else
  {
    printf("bpr reg => reset\r\n");
  }

  /* reset bpr domain */
  bpr_reset();

  /* write data to bpr dt register */
  bpr_reg_write();

  /* check if the written data are correct */
  if(bpr_reg_check() == TRUE)
  {
    printf("write bpr reg ok\r\n");
  }
  else
  {
    printf("write bpr reg fail\r\n");
  }

  /* disable pwc and bpr clock for power saving */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, FALSE);
  crm_periph_clock_enable(CRM_BPR_PERIPH_CLOCK, FALSE);

  while(1)
  {
  }
}

/**
  * @}
  */

/**
  * @}
  */
