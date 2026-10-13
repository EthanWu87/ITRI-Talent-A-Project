/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, shows how to use
  combine_mode(ordinary_simult).
  the convert data as follow:
  - adc1_ordinary_valuetab[0] ---> (adc2_channel_7<<16) | adc1_channel_4
  - adc1_ordinary_valuetab[1] ---> (adc2_channel_8<<16) | adc1_channel_5
  - adc1_ordinary_valuetab[2] ---> (adc2_channel_9<<16) | adc1_channel_6
