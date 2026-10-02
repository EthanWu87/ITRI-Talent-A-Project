/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the ht32f49395_sk board, in this demo, during normal
  operation, the wwdt count value is continuously reloaded in the main function,
  and wwdt reset will not occur at this time. when the wakeup button is pressed,
  the function stops reloading the wwdt count value, resulting in wwdt reset.
