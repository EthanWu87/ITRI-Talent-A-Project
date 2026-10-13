/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, shows how to use
  only receive mode receive data by polling mode.
  the pins connection as follow:
  - spi2 slaver               spi1 master
    pb12(cs)        <--->     pa4(cs)
    pb13(sck)       <--->     pa5(sck)
    pb15(mosi)      <--->     pa7(mosi)
