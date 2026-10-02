/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, provides a basic
  communication between usart2 and usart3 in half-duplex mode. if received data
  right, the three leds will blink.

  set-up
  - connect usart2 tx pin (pa2) to usart3 tx pin (pb10)
  - connect a pull-up resistor to this line (10k).
