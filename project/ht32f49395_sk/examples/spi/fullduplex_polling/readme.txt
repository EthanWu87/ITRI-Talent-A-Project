/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, shows how to use
  fullduplex mode transfer data by polling mode.
  the pins connection as follow:
  - spi2                      spi1
    pb12(cs)        <--->     pa4(cs)
    pb13(sck)       <--->     pa5(sck)
    pb14(miso)      <--->     pa6(miso)
    pb15(mosi)      <--->     pa7(mosi)
