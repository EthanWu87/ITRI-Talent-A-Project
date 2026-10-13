/*********************************************************************************************************//**
 * @file    ht32f493x5_crc.c
 * @version $Rev:: 128         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   contains all the functions for the crc firmware library
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

#include "ht32f493x5_conf.h"

/** @addtogroup HT32F493x5_periph_driver
  * @{
  */

/** @defgroup CRC
  * @brief CRC driver modules
  * @{
  */

#ifdef CRC_MODULE_ENABLED

/** @defgroup CRC_private_functions
  * @{
  */

/**
  * @brief  reset the crc data register.
  * @param  none
  * @retval none
  */
void crc_data_reset(void)
{
  /* reset crc generator */
  CRC->ctrl_bit.rst = 0x1;
}

/**
  * @brief  compute the 32-bit crc of a given data word(32-bit).
  * @param  data: data word(32-bit) to compute its crc
  * @retval 32-bit crc
  */
uint32_t crc_one_word_calculate(uint32_t data)
{
  CRC->dt = data;
  return (CRC->dt);
}

/**
  * @brief  compute the 32-bit crc of a given buffer of data word(32-bit).
  * @param  pbuffer: pointer to the buffer containing the data to be computed
  * @param  length: length of the buffer to be computed
  * @retval 32-bit crc
  */
uint32_t crc_block_calculate(uint32_t *pbuffer, uint32_t length)
{
  uint32_t index = 0;

  for(index = 0; index < length; index++)
  {
    CRC->dt = pbuffer[index];
  }

  return (CRC->dt);
}

/**
  * @brief  return the current crc value.
  * @param  none
  * @retval 32-bit crc
  */
uint32_t crc_data_get(void)
{
  return (CRC->dt);
}

/**
  * @brief  store a 8-bit data in the common data register.
  * @param  cdt_value: 8-bit value to be stored in the common data register
  * @retval none
  */
void crc_common_data_set(uint8_t cdt_value)
{
  CRC->cdt_bit.cdt = cdt_value;
}

/**
  * @brief  return the 8-bit data stored in the common data register
  * @param  none
  * @retval 8-bit value of the common data register
  */
uint8_t crc_common_data_get(void)
{
  return (CRC->cdt_bit.cdt);
}

/**
  * @brief  set the 32-bit initial data of crc
  * @param  value: initial data
  * @retval none
  */
void crc_init_data_set(uint32_t value)
{
  CRC->idt = value;
}

/**
  * @brief  control the reversal of the bit order in the input data
  * @param  value
  *         this parameter can be one of the following values:
  *         - CRC_REVERSE_INPUT_NO_AFFECTE
  *         - CRC_REVERSE_INPUT_BY_BYTE
  *         - CRC_REVERSE_INPUT_BY_HALFWORD
  *         - CRC_REVERSE_INPUT_BY_WORD
  * @retval none.
  */
void crc_reverse_input_data_set(crc_reverse_input_type value)
{
  CRC->ctrl_bit.revid = value;
}

/**
  * @brief  control the reversal of the bit order in the output data
  * @param  value
  *         this parameter can be one of the following values:
  *         - CRC_REVERSE_OUTPUT_NO_AFFECTE
  *         - CRC_REVERSE_OUTPUT_DATA
  * @retval none.
  */
void crc_reverse_output_data_set(crc_reverse_output_type value)
{
  CRC->ctrl_bit.revod = value;
}

/**
  * @brief  config crc polynomial value
  * @param  value
  *         32-bit new data of crc poly value
  * @retval none.
  */
void crc_poly_value_set(uint32_t value)
{
  CRC->poly = value;
}

/**
  * @brief  return crc polynomial value
  * @param  none
  * @retval 32-bit value of the polynomial value.
  */
uint32_t crc_poly_value_get(void)
{
  return (CRC->poly);
}

/**
  * @brief  config crc polynomial data size
  * @param  size
  *         this parameter can be one of the following values:
  *         - CRC_POLY_SIZE_32B
  *         - CRC_POLY_SIZE_16B
  *         - CRC_POLY_SIZE_8B
  *         - CRC_POLY_SIZE_7B
  * @retval none.
  */
void crc_poly_size_set(crc_poly_size_type size)
{
  CRC->ctrl_bit.poly_size = size;
}

/**
  * @brief  return crc polynomial data size
  * @param  none
  * @retval polynomial data size.
  */
crc_poly_size_type crc_poly_size_get(void)
{
  return (crc_poly_size_type)(CRC->ctrl_bit.poly_size);
}

/**
  * @}
  */

#endif

/**
  * @}
  */

/**
  * @}
  */
