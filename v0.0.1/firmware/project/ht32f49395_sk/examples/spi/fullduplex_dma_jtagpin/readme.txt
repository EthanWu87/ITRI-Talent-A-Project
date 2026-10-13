/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, shows how to use
  fullduplex mode transfer data by dma.
  the pins connection as follow:
  - spi2 slave                spi1 master
    pb12(cs)        <--->     pa15(cs)
    pb13(sck)       <--->     pb3(sck)
    pb14(miso)      <--->     pb4(miso)
    pb15(mosi)      <--->     pb5(mosi)
