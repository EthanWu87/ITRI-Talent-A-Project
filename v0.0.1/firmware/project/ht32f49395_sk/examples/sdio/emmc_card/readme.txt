/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, sdio1 to manage
  sd card test: init, erase, single block r/w, multiple blocks r/w.
  mmc card test:init, erase, single block r/w, multiple blocks r/w, stream data r/w.

    sdio1                                           sd/mmc card
  - sdio1_d0                    pc8          --->   dat0
  - sdio1_d1                    pc9          --->   dat1
  - sdio1_d2                    pc10         --->   dat2
  - sdio1_d3                    pc11         --->   dat3
  - sdio1_ck                    pc12         --->   clk
  - sdio1_cmd                   pd2          --->   cmd
