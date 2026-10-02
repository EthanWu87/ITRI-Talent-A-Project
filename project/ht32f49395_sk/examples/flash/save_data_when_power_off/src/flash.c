/*********************************************************************************************************//**
 * @file    flash.c
 * @version $Rev:: 148         $
 * @date    $Date:: 2025-05-08 #$
 * @brief   flash program
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

#include "flash.h"
#include <stdio.h>

uint32_t sector_size = 0;

/* max 2k byte */
uint16_t flash_buf[2048/2];

/**
  * @brief  flash read halfword function.
  * @param  faddr: flash address.
  * @retval the data of assign address
  */
uint16_t flash_readhalfword(uint32_t faddr)
{
  return *(uint16_t*)faddr;
}

/**
  * @brief  flash write without check function.
  * @param  writeaddr: address start.
  * @param  pbuffer: data point.
  * @param  numtowrite: halfword data count.
  * @retval none
  */
void flash_write_nocheck(uint32_t writeaddr, uint16_t *pbuffer, uint16_t numtowrite)
{
  uint16_t index;
  for(index = 0; index < numtowrite; index++)
  {
    flash_halfword_program(writeaddr, pbuffer[index]);

    /* address add 2 */
    writeaddr += 2;
  }
} 

/**
  * @brief  flash write function.
  * @param  writeaddr: address start(address must be even).
  * @param  pbuffer: data point.
  * @param  numtowrite: halfword data count.
  * @retval none
  */
void flash_write(uint32_t writeaddr, uint16_t *pbuffer, uint16_t numtowrite)
{
  /* sector address */
  uint32_t secpos;

  /* intra sector offset address */
  uint16_t secoff;

  /* remaining address in sector */
  uint16_t secremain;

  uint16_t index;
  uint32_t offaddr;

  if(FLASH_SIZE<256)
    sector_size = 1024 ;
  else
    sector_size	= 2048;

  if(writeaddr<FLASH_BASE+FLASH_CODE_SIZE*1024||(writeaddr>=(FLASH_BASE+1024*FLASH_SIZE)))

  /* illegal address direct return */
  return;

  /* unlock the flash controller */
  flash_unlock();

  offaddr = writeaddr - FLASH_BASE;
  secpos = offaddr / sector_size;
  secoff = (offaddr % sector_size) / 2;
  secremain = sector_size / 2 - secoff;
  if(numtowrite <= secremain)
    /* not greater than the sector range */
    secremain=numtowrite;
  while(1) 
  {
    flash_read(secpos * sector_size + FLASH_BASE, flash_buf, sector_size / 2);
    for(index = 0; index < secremain; index++)
    {
      /* data check */
      if(flash_buf[secoff + index] != 0xFFFF)
        /* erase required */
        break;
    }
    if(index < secremain)
    {
      /* erase this sector */
      flash_sector_erase(secpos * sector_size + FLASH_BASE);
      for(index = 0; index < secremain; index++)
      {
        flash_buf[index + secoff] = pbuffer[index];	
      }
      /* write this sector */
      flash_write_nocheck(secpos * sector_size + FLASH_BASE, flash_buf, sector_size / 2);
    }
    else
    {
      /* erased, write directly to the remaining section of the sector */
      flash_write_nocheck(writeaddr, pbuffer, secremain);
    }
    if(numtowrite == secremain)
      break;
    else
    {
      /* sector address + 1 */
      secpos++;

      /* offset position is 0 */
      secoff = 0;

      /* pointer offset */
      pbuffer += secremain;

      /* write address offset */
      writeaddr += (secremain * 2);

      /* halfword decrement */
      numtowrite -= secremain;
      if(numtowrite > (sector_size / 2))
        /* the next sector is still not finished */
        secremain = sector_size / 2;
      else
        /* the next sector can be written */
        secremain = numtowrite;
    }
  }

  /* lock the flash controller */
  flash_lock();
}

/**
  * @brief  flash read function.
  * @param  readaddr: address start(address must be even).
  * @param  pbuffer: data point.
  * @param  numtoread: halfword data count.
  * @retval none
  */
void flash_read(uint32_t readaddr, uint16_t *pbuffer, uint16_t numtoread)
{
  uint16_t index;

  for(index = 0; index < numtoread; index++)
  {
    pbuffer[index] = flash_readhalfword(readaddr);

    /* address add 2 */
    readaddr += 2;
  }
}

