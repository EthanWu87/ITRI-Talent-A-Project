/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo,shows how to use spi access
  the w25q flash chip.
  the pins use as follow:
  - cs        <--->   pb12(software cs, pb12 as a general io to control flash cs)
  - sck       <--->   pb13
  - miso      <--->   pb14
  - mosi      <--->   pb15
  - usart1_tx <--->   pa9
