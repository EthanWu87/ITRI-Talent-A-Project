/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, use hardware i2c2 
  write or read data based on the eeprom device. if the communication is
  successful, led3 will turn on, if the communication fails, led2 will keep flashing.

  attention:
    1. i2c bus must pull-up
    2. press the slave button first, then press the master button to start communication.

  pin used:
    1. scl --- pb10
    2. sda --- pb11
