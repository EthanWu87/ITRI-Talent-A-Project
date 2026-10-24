/*********************************************************************************************************//**
 * @file    BMC81M001_HT32/src/BMC81M001.c
 * @version V1.0.1
 * @date    2024-10-10
 * @brief   The function of BMC81M001 driver.
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

/* Includes ------------------------------------------------------------------------------------------------*/
#include "BMC81M001.h"
#include "flow_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include "ht32f493x5_int.h"

/* Settings ------------------------------------------------------------------------------------------------*/
/* Private types -------------------------------------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------------------------------------*/
static flag_status BMC81M001_uartByteAvailable(void);
static void BMC81M001_uartReadByte(uint8_t *data);
static void BMC81M001_uartWrite(const uint8_t *data, uint16_t length);
/* Private macro -------------------------------------------------------------------------------------------*/
/* Global variables ----------------------------------------------------------------------------------------*/
usart_type *gBMC81M001_SERIAL = BMC81M001_USART_PORT;
uint8_t BMC81M001_dataBuff[25];
/* Private variables ---------------------------------------------------------------------------------------*/
uint8_t _BMC81M001_len;     //Length of data reveviced
uint8_t _BMC81M001_state;   
uint8_t _BMC81M001_sum;
uint8_t _BMC81M001_checksum;//Checksum of data reveviced     
uint8_t _BMC81M001_res;
char BMC81M001_Response[RES_MAX_LENGTH];
int resLength = 0;
uint8_t uselessBuff[200];
char OneNetReciveBuff[200];
static volatile uint8_t BMC81M001_rxBuffer[BMC81M001_RX_BUFFER_SIZE];
static volatile uint16_t BMC81M001_rxHead;
static volatile uint16_t BMC81M001_rxTail;
/* Global functions ----------------------------------------------------------------------------------------*/

/*********************************************************************************************************//**
 * @brief module serial number select.
 * @param serial_number: select serial number.
 * @retval BMC81M001_FAILURE/BMC81M001_SUCCESS
 ************************************************************************************************************/
BMC81M001_selStatus BMC81M001_selSerial(usart_type *serial_port)
{
  if(serial_port == NULL)
  {
    return BMC81M001_FAILURE;
  }
  gBMC81M001_SERIAL = serial_port;
  return BMC81M001_SUCCESS;
}

/*********************************************************************************************************//**
 * @brief  Module initialization using UART communication.
* @param  BaudRate:Development board communication speed
 * @retval void
 ************************************************************************************************************/
void BMC81M001_Init(uint32_t BaudRate)
{
	gpio_init_type gpio_init_struct;

  BMC81M001_rxHead = 0;
  BMC81M001_rxTail = 0;

  /* Enable UART4 and GPIO clocks. */
  crm_periph_clock_enable(BMC81M001_USART_CLK, TRUE);
  crm_periph_clock_enable(BMC81M001_USART_TX_GPIO_CLK, TRUE);
	
  /* Configure the UART4 TX pin. */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = BMC81M001_USART_TX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(BMC81M001_USART_TX_GPIO_PORT, &gpio_init_struct);
	
  /* Configure the UART4 RX pin. */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = BMC81M001_USART_RX_PIN;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;
  gpio_init(BMC81M001_USART_RX_GPIO_PORT, &gpio_init_struct);

  /* Configure the UART4 interrupt. */
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  nvic_irq_enable(BMC81M001_USART_IRQn, 0, 0);

  usart_init(BMC81M001_USART_PORT, BaudRate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_transmitter_enable(BMC81M001_USART_PORT, TRUE);
  usart_receiver_enable(BMC81M001_USART_PORT, TRUE);

  /* Enable UART4 receive interrupt. */
  usart_interrupt_enable(BMC81M001_USART_PORT, USART_RDBF_INT, TRUE);
  usart_enable(BMC81M001_USART_PORT, TRUE);
}
/*********************************************************************************************************//**
 * @brief  Connect to AP network.
* @param   ssid:The ID of the AP network
					 pass:Network password
* @retval Communication status
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_connectToAP(char ssid[],char pass[])
{  
  char  cmd[200] = "AT+CWJAP=\"";
  /* <AT+CWMODE=1> command to Set station mode   */
  if( BMC81M001_sendATCommand("AT+CWMODE=1\r\n",3) ==  BMC81M001_SEND_SUCCESS ) ;
  else 
  {
    return false;
  }
  /* <AT+CWJAP="ssid","password"> add to AP  */
	BMC81M001_arrayAdd(cmd,ssid);
  BMC81M001_arrayAdd(cmd,"\",\"");
  BMC81M001_arrayAdd(cmd,pass);
  BMC81M001_arrayAdd(cmd,"\"\r\n");
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS ) 
  {  
    return true;
  }
  else 
  {
    return false;
  } 
}
/*********************************************************************************************************//**
 * @brief Connect to TCP server
* @param  ip:IP address for connecting to TCP server
					port:Connection port
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_connectTCP(char ip[],int port)
{  
  char  cmd[200] = "AT+CIPSTART=\"TCP\",\"";
  char  num[5];
	BMC81M001_arrayAdd(cmd,ip);
	BMC81M001_arrayAdd(cmd,"\",");
	sprintf(num,"%d", port);
	BMC81M001_arrayAdd(cmd,num);
  BMC81M001_arrayAdd(cmd,"\r\n");
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS )
  {
    return true;
  }
  else 
  {
    return false;
  } 
}
/*********************************************************************************************************//**
* @brief Sending data to TCP server
* @param  Dlength:Data length
					Dbuffer:Data to be sent to TCP server
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_writeDataTcp(int Dlength,char Dbuffer[])
{
  char  cmd[200] = "AT+CIPSEND=";
  char  num[2];
	sprintf(num,"%d", Dlength);
	BMC81M001_arrayAdd(cmd,num);
	BMC81M001_arrayAdd(cmd,"\r\n");
  if(BMC81M001_sendATCommand(cmd,4) == BMC81M001_SEND_SUCCESS ) ;
  else 
  {
    return false;
  } 
  _BMC81M001_delay(100);
  if(BMC81M001_sendATCommand(Dbuffer,3) == BMC81M001_SEND_SUCCESS )  
  {  
    return true;
  }
  else 
  {
    return false;
  } 
}
/*********************************************************************************************************//**
 * @brief  Read data issued by TCP server
* @param   readbuff:Data to be read to TCP server
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_readDataTcp(char readbuff[])
{
  int index;  
	char *position;
	int i,length=0;
	BMC81M001_clearResponse(BMC81M001_Response);
  if(BMC81M001_uartByteAvailable() == SET)
	{
		BMC81M001_readBytes((u8*)BMC81M001_Response,200,3);
	}
  if(strstr(BMC81M001_Response, "+IPD") != NULL)
  {
		length=strlen(BMC81M001_Response);
		position = strchr(BMC81M001_Response,':');
		index = position - BMC81M001_Response;
		length=length-index;
		for(i=0;i<length;i++)
		{
			readbuff[i]=BMC81M001_Response[i+index+1];
		}
		return true;
  }
	return false;
}
/*********************************************************************************************************//**
 * @brief  Configure MQTT connection parameters
* @param   clientlid,username,password,mqtt_host,server_port 
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_configMqtt(char clientid[],char username[],char password[],char mqtt_host[],int server_port)
{
/**********************************************************  
Description: Set mqtt user properties
command:     At+mqttusercfg=<linkid>, <scheme>, < "client\u ID" >, <username ">, <password" >, <cert_ Key_ Id>, <ca_ Id>, < "path" >
**********************************************************/
  char  cmd[200]="AT+MQTTUSERCFG=0,1,\"NULL\",\"NULL\",\"NULL\",0,0,\"\"\r\n";
	
  char  cmdCLIENTID[200]="AT+MQTTCLIENTID=0,\"";
  char  cmdUSERNAME[100]="AT+MQTTUSERNAME=0,\"";
  char  cmdPASSWORD[200]="AT+MQTTPASSWORD=0,\"";
	char  num[5];
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS ) ;
  else 
  {
    return false;
  }   
/**********************************************************
Description: Set mqtt client ID
command:     AT+MQTTCLIENTID=<LinkID>,<"client_id">
**********************************************************/
  BMC81M001_arrayAdd(cmdCLIENTID,clientid);
  BMC81M001_arrayAdd(cmdCLIENTID,"\"\r\n");
  if(BMC81M001_sendATCommand(cmdCLIENTID,3) == BMC81M001_SEND_SUCCESS ) ;
  else 
  {
    return false;
  }   
/**********************************************************
Description: Set mqtt username
command:     AT+MQTTUSERNAME=<LinkID>,<"username">
**********************************************************/
  BMC81M001_arrayAdd(cmdUSERNAME,username);
  BMC81M001_arrayAdd(cmdUSERNAME,"\"\r\n");
  if(BMC81M001_sendATCommand(cmdUSERNAME,3) == BMC81M001_SEND_SUCCESS ) ;
  else 
  {
    return false;
  } 
/**********************************************************
Description: Set mqtt userpassword
command:     AT+MQTTPASSWORD=<LinkID>,<"password">
**********************************************************/
  BMC81M001_arrayAdd(cmdPASSWORD,password);
  BMC81M001_arrayAdd(cmdPASSWORD,"\"\r\n");
  if(BMC81M001_sendATCommand(cmdPASSWORD,3) == BMC81M001_SEND_SUCCESS) ;
  else 
  {
    return false;
  } 
/**********************************************************
Description: Connect  MQTT Broker
command:     AT+MQTTCONN=<LinkID>,<"host">,<port>,<reconnect>
**********************************************************/
  memset(cmd,'\0',200);
	strcpy(cmd,"AT+MQTTCONN=0,\"");
	BMC81M001_arrayAdd(cmd,mqtt_host);
  BMC81M001_arrayAdd(cmd,"\",");
	sprintf(num,"%d",server_port);
	BMC81M001_arrayAdd(cmd,num);
  BMC81M001_arrayAdd(cmd,",0\r\n");
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS )
  {  
    return true;
  }
  else 
  {
    return false;
  } 
}
/*********************************************************************************************************//**
 * @brief  Subscribe to publish relevant TOPIC
* @param   publishtopic:Topic of platform release
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_setPublishTopic(char publishtopic[])
{
  char  cmd[200]="AT+MQTTSUB=0,\"";
	BMC81M001_arrayAdd(cmd,publishtopic);
  BMC81M001_arrayAdd(cmd,"\",0\r\n");
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS )
  {  
    return true;
  }
  else 
  {
    return false;
  } 
}
/*********************************************************************************************************//**
 * @brief  Subscription topics on the platform
* @param   subscribetopic:Topic of Subscribe
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_setSubscribetopic(char subscribetopic[])
{
  char  cmd[200]="AT+MQTTSUB=0,\"";
	BMC81M001_arrayAdd(cmd,subscribetopic);
  BMC81M001_arrayAdd(cmd,"\",0\r\n");
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS )
  {  
    return true;
  }  
  else 
  {
    return false;
  } 
}
/*********************************************************************************************************//**
 * @brief  Set custom TOPIC
* @param   topic:custom TOPIC
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_setTopic(char topic[])
{
  char  cmd[200]="AT+MQTTSUB=0,\"";
	BMC81M001_arrayAdd(cmd,topic);
  BMC81M001_arrayAdd(cmd,"\",0\r\n");
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS )
  {  
    return true;
  }  
  else 
  {
    return false;
  } 
}
/*********************************************************************************************************//**
* @brief Send data to Internet platform
* @param  Dbuffer:custom TOPIC
					Dlength:Data length
					topic:Target topic for sending
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_writeByte(char Dbuffer[],int Dlength,char topic[])
{
  char  cmd[200] = "AT+MQTTPUBRAW=0,\"";
	char  num[2];
  BMC81M001_arrayAdd(cmd,topic);
  BMC81M001_arrayAdd(cmd,"\",");
	sprintf(num,"%d", Dlength);
  BMC81M001_arrayAdd(cmd,num);
  BMC81M001_arrayAdd(cmd,",0,0\r\n");
  if(BMC81M001_sendATCommand(cmd,3) == BMC81M001_SEND_SUCCESS )
	{
		BMC81M001_sendATCommand(Dbuffer,3);
	}
  else 
  {
    return false;
  } 	
  return true;
}
/*********************************************************************************************************//**
* @brief  read data to Internet platform
* @param  IotReciveBuff:read data
					*IotReciveBufflen:Data length
					topic:Data source topic
* @retval void
 ************************************************************************************************************/
void BMC81M001_readIotData(char IotReciveBuff[],int *IotReciveBufflen,char IotReciveTopic[])
{
	uint8_t i =0;
  uint8_t receivedByte;
	int position=0;	
	int length=0;
	*IotReciveBufflen=0;
  BMC81M001_clearResponse(BMC81M001_Response);
  if(BMC81M001_uartByteAvailable() == SET)
	{
		_BMC81M001_delay(10);
    while(BMC81M001_uartByteAvailable() == SET)
		{
      if(resLength < (RES_MAX_LENGTH - 1))
			{
        BMC81M001_uartReadByte((uint8_t *)BMC81M001_Response + resLength);
        resLength++;
        BMC81M001_Response[resLength] = '\0';
      }
      else
      {
        BMC81M001_uartReadByte(&receivedByte);
        BMC81M001_clearResponse(BMC81M001_Response);
			}
			if(strstr(BMC81M001_Response,"\r\n") != NULL)
			{
				break;
			}
		}
	}
  if(strstr(BMC81M001_Response,"+MQTTSUBRECV") != NULL)
  {
		position = BMC81M001_arrayFind(BMC81M001_Response,'"');
		length=strlen(BMC81M001_Response);
    for(i=0;i<length-position;i++)
		{
      BMC81M001_Response[i]=BMC81M001_Response[i+position+1];;
			OneNetReciveBuff[i]=BMC81M001_Response[i+position];;  
		}	
		position = BMC81M001_arrayFind(BMC81M001_Response,'"');
		length=strlen(BMC81M001_Response);
    for( i=0;i<position;i++)
		{
			IotReciveTopic[i]=BMC81M001_Response[i];
		}
    for( i=0;i<length-position;i++)
		{
			IotReciveBuff[i]=BMC81M001_Response[i+position+2];
		}
		position = BMC81M001_arrayFind(IotReciveBuff,',');
		*IotReciveBufflen=IotReciveBuff[position-1];
		length=strlen(IotReciveBuff);
    for( i=0;i<length-position;i++)
		{
			IotReciveBuff[i]=IotReciveBuff[i+position+1];
		}
	}		
}
/*********************************************************************************************************//**
* @brief  BMC81M001 write Bytes with UART
* @param  wbuf:Data that needs to be written
* @retval void
 ************************************************************************************************************/
void BMC81M001_writeBytes(char wbuf[])
{
		uint8_t len = 0;
		uint8_t i = 0;
		len=strlen(wbuf);
		if(len>128)
		{
			for(i=0;i<len-125;i++)
			{
				OneNetReciveBuff[i]=wbuf[125+i];
			}
      BMC81M001_uartWrite((const uint8_t *)wbuf,125);
			_BMC81M001_delay(2);
      BMC81M001_uartWrite((const uint8_t *)OneNetReciveBuff,len-125);
			BMC81M001_clearResponse(OneNetReciveBuff);
		}
		else
		{		
      BMC81M001_uartWrite((const uint8_t *)wbuf,len);
		}
}
/*********************************************************************************************************//**
* @brief  Reset module
* @param  void
* @retval Communication status  
					@arg true:SEND_Success 
					@arg false:SEND_FAIL 
 ************************************************************************************************************/
bool BMC81M001_reset(void)
{  
  if(BMC81M001_sendATCommand("AT+RST\r\n",3) == BMC81M001_SEND_SUCCESS )
  {
    BMC81M001_clearResponse(BMC81M001_Response);
    _BMC81M001_delay(2000) ;        // Issue soft-reset command
  }
  else 
  {
			return false;
  }
  return true;
}
/*********************************************************************************************************//**
 * @brief  Send AT related instructions
 * @param  @arg StringstrCmd:Instructions that need to be sent to the module
					 @arg reTry:Repeated sending times
 * @retval BMC81M001_SEND_FAIL/BMC81M001_SEND_SUCCESS
 ************************************************************************************************************/

int BMC81M001_sendATCommand(char StringstrCmd[],uint8_t reTry)
{
  char response[]="OK";
	uint8_t time=0;
	uint8_t i=0;
	uint8_t tryTime = 0;
  for(tryTime = 0; tryTime < reTry; tryTime++)
  {  
		time=0;
		BMC81M001_readBytes(uselessBuff,10,1);
		BMC81M001_clearResponse(BMC81M001_Response);
		BMC81M001_writeBytes(StringstrCmd);
  /* check the reply message within the timeout period. If the reply message contains preset characters,  return success  */
    for(i=0;;i++)
    {
			time++;
		  BMC81M001_readBytes((u8*)BMC81M001_Response,100,time);		
      if(strstr(BMC81M001_Response,response) != NULL)
      {	
        return BMC81M001_SEND_SUCCESS;
      }		
      if(time==4)
      {
        break;
      }      
		}
	}
	return BMC81M001_SEND_FAIL;
}
/*********************************************************************************************************//**
* @brief  read data with UART
* @param  rbuf:data
					timeout:Waiting for timeout
					reTry:Repeated sending times
* @retval void
 ************************************************************************************************************/
void BMC81M001_readBytes(uint8_t rbuf[],int timeout, uint8_t reTry)
{
	unsigned char tryTime = 0;
	uint32_t _delayCount = 0;
  uint16_t bufferIndex;
  char response[]="OK";

  bufferIndex = (rbuf == (uint8_t *)BMC81M001_Response) ? (uint16_t)resLength : 0;
  for(tryTime = 0; tryTime < reTry; tryTime++)
{
	for(_delayCount = 0; _delayCount<timeout; _delayCount++)
	{
    while(BMC81M001_uartByteAvailable() == SET)
		{
      if(bufferIndex < (RES_MAX_LENGTH - 1))
			{
        BMC81M001_uartReadByte(rbuf + bufferIndex);
        bufferIndex++;
        if(rbuf == (uint8_t *)BMC81M001_Response)
        {
          resLength = (int)bufferIndex;
          BMC81M001_Response[resLength] = '\0';
        }
      }
      else
      {
        BMC81M001_uartReadByte(&uselessBuff[0]);
			}
      if((rbuf == (uint8_t *)BMC81M001_Response) &&
         (strstr(BMC81M001_Response,response) != NULL))
      {	
				break;
      }	
		}
		_BMC81M001_delay(1);
	}
}
}

/* Private functions ---------------------------------------------------------------------------------------*/
static flag_status BMC81M001_uartByteAvailable(void)
{
  return (BMC81M001_rxHead != BMC81M001_rxTail) ? SET : RESET;
}

static void BMC81M001_uartReadByte(uint8_t *data)
{
  if(BMC81M001_rxHead != BMC81M001_rxTail)
  {
    *data = BMC81M001_rxBuffer[BMC81M001_rxTail];
    BMC81M001_rxTail = (uint16_t)((BMC81M001_rxTail + 1U) % BMC81M001_RX_BUFFER_SIZE);
  }
}

bool BMC81M001_readByte(uint8_t *data)
{
  if(data == NULL || BMC81M001_uartByteAvailable() == RESET)
  {
    return false;
  }
  BMC81M001_uartReadByte(data);
  return true;
}

void BMC81M001_RxIrqHandler(void)
{
  uint8_t data;
  uint16_t nextHead;

  while(usart_flag_get(BMC81M001_USART_PORT, USART_RDBF_FLAG) != RESET)
  {
    data = (uint8_t)usart_data_receive(BMC81M001_USART_PORT);
    nextHead = (uint16_t)((BMC81M001_rxHead + 1U) % BMC81M001_RX_BUFFER_SIZE);

    if(nextHead != BMC81M001_rxTail)
    {
      BMC81M001_rxBuffer[BMC81M001_rxHead] = data;
      BMC81M001_rxHead = nextHead;
      uint32_t used = (nextHead + BMC81M001_RX_BUFFER_SIZE - BMC81M001_rxTail) % BMC81M001_RX_BUFFER_SIZE;
      if(used > g_flow_metrics.wifi_rx_peak) g_flow_metrics.wifi_rx_peak = used;
    }
    else ++g_flow_metrics.wifi_rx_drops;
  }
}

static void BMC81M001_uartWrite(const uint8_t *data, uint16_t length)
{
  uint16_t index;

  for(index = 0; index < length; index++)
  {
    while(usart_flag_get(gBMC81M001_SERIAL, USART_TDBE_FLAG) == RESET)
    {
    }
    usart_data_transmit(gBMC81M001_SERIAL, data[index]);
  }
}

/*********************************************************************************************************//**
 * @brief  clear Buff.
 * @param  void
 * @retval void
 ************************************************************************************************************/
void _BMC81M001_clearBuf(void)
{
	int a = 0;
  for(a = 0; a < 12; a++)
  {
  } 
}

/*********************************************************************************************************//**
 * @brief  delay ms.
 * @param  count
 * @retval void
 ************************************************************************************************************/
void _BMC81M001_delay(vu32 count)
{
  count = SystemCoreClock / 8000 * count;
  while(count--);
}


/**********************************************************
 * @brief  clear Response
* @param  Dbuffer:Array that needs to be cleared
 * @retval void      
**********************************************************/
void BMC81M001_clearResponse(char Dbuffer[])
{
  memset(Dbuffer,'\0',RES_MAX_LENGTH);
  resLength = 0;
}
/**********************************************************
 * @brief  add two array
* @param  array1,array2
 * @retval void       
**********************************************************/
void BMC81M001_arrayAdd(char array1[],char array2[])
{
	int length1;
	int length2;
	int i=0;
	length1=strlen(array1);
	length2=strlen(array2);
	for(i=0;i<length2;i++)
	{
	  array1[i+length1]=array2[i];
	}
}
/**********************************************************
 * @brief charbuff add
* @param  Search for characters in array one and return the sequence of the first occurrence
 * @retval position      
**********************************************************/
int BMC81M001_arrayFind(char array1[],char array2)
{
	int length;
	int i=0;
	int position=0;
	length=strlen(array1);
	for(i=0;i<length;i++)
	{
	  if(array1[i]==array2)
		{
			position=i;
			i=length;
		}
	}
	return position;
}

// BEGIN BMC NONBLOCKING TCP
#define WIFI_CHUNK_SIZE 2048U
#define WIFI_TX_MAX 256U
#define EVENT_MAX BMC81M001_CONTROL_MAX
#define EVENT_COUNT BMC81M001_CONTROL_COUNT
#define WIFI_TX_TIMEOUT_MS 1000U
/* DMA1 Channel3 is reserved for UART4 TX. Camera/image TX use Channels2/1.
 * Short AT commands are copied; large chunks borrow the retained packet until
 * SEND OK. Cancelling DMA always precedes releasing or reusing queue storage.
 */
typedef struct
{
  uint8_t command[WIFI_TX_MAX];
  uint8_t active, failed;
} bmc_tcp_tx_t;
static bmc_tcp_tx_t wifi_tx;

static void bmc_tcp_dma_init(void)
{
  dma_init_type config;
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  dma_reset(DMA1_CHANNEL3);
  dma_default_para_init(&config);
  config.peripheral_base_addr = (uint32_t)&BMC81M001_USART_PORT->dt;
  config.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  config.peripheral_inc_enable = FALSE;
  config.memory_inc_enable = TRUE;
  config.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  config.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  config.loop_mode_enable = FALSE;
  config.priority = DMA_PRIORITY_LOW;
  dma_init(DMA1_CHANNEL3, &config);
  dma_flexible_config(DMA1, FLEX_CHANNEL3, DMA_FLEXIBLE_UART4_TX);
}

static void bmc_tcp_tx_cancel(usart_type *port, bmc_tcp_tx_t *tx)
{
  usart_dma_transmitter_enable(port, FALSE);
  dma_channel_enable(DMA1_CHANNEL3, FALSE);
  dma_flag_clear(DMA1_GL3_FLAG);
  tx->active = tx->failed = 0U;
}

static uint8_t bmc_tcp_tx_bytes(usart_type *port, bmc_tcp_tx_t *tx,
                               const uint8_t *data, size_t length)
{
  if(data == NULL || length == 0U || length > WIFI_CHUNK_SIZE || tx->active)
    return 0U;
  if(length <= sizeof(tx->command))
  {
    memcpy(tx->command, data, length);
    data = tx->command;
  }
  dma_channel_enable(DMA1_CHANNEL3, FALSE);
  dma_flag_clear(DMA1_GL3_FLAG);
  DMA1_CHANNEL3->maddr = (uint32_t)data;
  dma_data_number_set(DMA1_CHANNEL3, (uint16_t)length);
  tx->failed = 0U;
  tx->active = 1U;
  __DMB();
  dma_channel_enable(DMA1_CHANNEL3, TRUE);
  usart_dma_transmitter_enable(port, TRUE);
  return 1U;
}

static uint8_t bmc_tcp_tx_done(usart_type *port, bmc_tcp_tx_t *tx)
{
  if(!tx->active) return !tx->failed;
  if(dma_flag_get(DMA1_DTERR3_FLAG) != RESET)
  {
    bmc_tcp_tx_cancel(port, tx);
    tx->failed = 1U;
    return 0U;
  }
  if(dma_flag_get(DMA1_FDT3_FLAG) == RESET) return 0U;
  usart_dma_transmitter_enable(port, FALSE);
  dma_channel_enable(DMA1_CHANNEL3, FALSE);
  if(usart_flag_get(port, USART_TDC_FLAG) == RESET) return 0U;
  dma_flag_clear(DMA1_GL3_FLAG);
  tx->active = 0U;
  return 1U;
}

/* Wi-Fi parser: ESP8266 +IPD,<length>:<data>. Handles fragmentation.
 * Only AT command replies are recorded outside +IPD payload.
 */
static char wifi_at[512];
static uint16_t at_n = 0;
static char ipd_head[30];
static uint8_t ipd_hn = 0, ipd_mode = 0;
/* 0 normal, 1 header, 2 payload */
static uint16_t ipd_remaining = 0;
static uint8_t ipd_match;
static uint8_t wifi_connected;
static BMC81M001_TcpCallbacks tcp_callbacks;
static void bmc_tcp_at_byte(char b)
{
  if (at_n >= sizeof(wifi_at) -1U)
  {
    memmove(wifi_at, wifi_at + 128U, sizeof(wifi_at) -129U);
    at_n = (uint16_t) (sizeof(wifi_at) -129U);
  }
  wifi_at[at_n++] = b;
  wifi_at[at_n] = '\0';
}

static void bmc_tcp_byte(uint8_t b)
{
  const char * prefix = "+IPD,";
  if (ipd_mode == 2)
  {
    if(tcp_callbacks.receive_byte) tcp_callbacks.receive_byte(b);
    if (--ipd_remaining == 0U) ipd_mode = 0;
    return;
  }
  if (ipd_mode == 1)
  {
    if (b == ':')
    {
      unsigned long n;
      ipd_head[ipd_hn] = 0;
      /* CIPMUX=0: +IPD,len; optional +IPD,0,len accepted. */
      {
        char * p = strrchr(ipd_head, ',');
        n = strtoul(p ? p + 1 : ipd_head, NULL, 10);
      }
      ipd_remaining = (uint16_t) ((n > 4096U) ? 0U : n);
      ipd_mode = (ipd_remaining > 0U) ? 2U : 0U;
      ipd_hn = 0;
      return;
    }
    if (ipd_hn < sizeof(ipd_head) -1U && ((b >= '0' && b <= '9') || b == ','))
    {
      ipd_head[ipd_hn++] = (char) b;
      return;
    }
    /* Malformed header: discard and recover. */
    ipd_mode = 0;
    ipd_hn = 0;
  }
  /* Scan normal text for +IPD, while retaining AT responses. */
  {
    if (b == (uint8_t) prefix[ipd_match])
    {
      ipd_match++;
      if (ipd_match == 5U)
      {
        ipd_mode = 1;
        ipd_hn = 0;
        ipd_match = 0;
        return;
      }
    }
    else ipd_match = (b == '+') ? 1U : 0U;
  }
  bmc_tcp_at_byte((char) b);
}

static void bmc_tcp_poll(void)
{
  uint8_t b;
  uint16_t budget = BMC81M001_RX_BUFFER_SIZE;
  while(budget-- != 0U && BMC81M001_readByte(&b))
  {
    bmc_tcp_byte((uint8_t)b);
  }
}

static void bmc_tcp_reset_at_capture(void)
{
  at_n = 0;
  wifi_at[0] = 0;
}

static uint8_t bmc_tcp_result_line(const char * response, const char * word)
{
  const char * p = response;
  size_t n = strlen(word);
  while (* p)
  {
    const char * start = p;
    while (* p && * p != '\r' && * p != '\n') p++;
    if (*p != '\0' && (size_t) (p - start) == n && strncmp(start, word, n) == 0) return 1;
    while (* p == '\r' || * p == '\n') p++;
  }
  return 0;
}


typedef enum
{
  WIFI_START, WIFI_TX, WIFI_REPLY, WIFI_ONLINE, WIFI_BACKOFF
} bmc_tcp_state_t;

typedef enum
{
  WIFI_CONNECT, WIFI_SEND_PROMPT, WIFI_SEND_ACK, WIFI_CLOSE
} bmc_tcp_operation_t;

static bmc_tcp_state_t wifi_state;
static bmc_tcp_operation_t wifi_operation;
static uint8_t wifi_step;
static uint32_t wifi_since, wifi_timeout;
static uint8_t events[EVENT_COUNT][EVENT_MAX];
static uint16_t event_lengths[EVENT_COUNT];
static uint8_t event_head, event_tail, event_count;


typedef struct
{
  uint8_t data[BMC81M001_PACKET_MAX];
  uint16_t length;
  uint32_t token;
} packet_t;
static packet_t packets[BMC81M001_PACKET_COUNT];
static uint8_t packet_head, packet_tail, packet_count;
typedef enum { SEND_NONE, SEND_CONTROL, SEND_PACKET } send_kind_t;
static send_kind_t send_kind;
static uint16_t send_offset, send_chunk;
static uint8_t wifi_close_required;

uint8_t BMC81M001_QueuePacket(const BMC81M001_DataSpan *parts, uint8_t count, uint32_t token)
{
  uint32_t total = 0U;
  uint8_t i;
  packet_t *packet;
  if(parts == NULL || count == 0U || packet_count >= BMC81M001_PACKET_COUNT) return 0U;
  for(i = 0U; i < count; ++i)
  {
    if(parts[i].data == NULL || parts[i].length == 0U) return 0U;
    total += parts[i].length;
    if(total > BMC81M001_PACKET_MAX) return 0U;
  }
  packet = &packets[packet_tail];
  total = 0U;
  for(i = 0U; i < count; ++i)
  {
    memcpy(packet->data + total, parts[i].data, parts[i].length);
    total += parts[i].length;
  }
  packet->length = (uint16_t)total;
  packet->token = token;
  packet_tail = (uint8_t)((packet_tail + 1U) % BMC81M001_PACKET_COUNT);
  ++packet_count;
  return 1U;
}

static const uint8_t *bmc_tcp_send_data(void)
{
  return send_kind == SEND_PACKET ? packets[packet_head].data :
                                (const uint8_t *)events[event_head];
}

static uint16_t bmc_tcp_send_length(void)
{
  return send_kind == SEND_PACKET ? packets[packet_head].length :
                                event_lengths[event_head];
}


uint8_t BMC81M001_QueueData(const uint8_t *data, uint16_t length)
{
  if(!wifi_connected || event_count >= EVENT_COUNT || data == NULL ||
     length == 0U || length > EVENT_MAX) return 0U;
  memcpy(events[event_tail], data, length);
  event_lengths[event_tail] = length;
  event_tail = (uint8_t)((event_tail + 1U) % EVENT_COUNT);
  ++event_count;
  return 1U;
}

uint8_t BMC81M001_ControlSlotsFree(void) { return EVENT_COUNT - event_count; }

static void bmc_tcp_disconnect(uint32_t now)
{
  bmc_tcp_tx_cancel(BMC81M001_USART_PORT, &wifi_tx);
  /* A retry must begin on a fresh TCP stream, never inside a partial record. */
  if(send_kind != SEND_NONE) wifi_close_required = 1U;
  send_kind = SEND_NONE;
  send_offset = send_chunk = 0U;
  wifi_connected = 0U;
  ipd_mode = ipd_hn = ipd_match = 0U;
  ipd_remaining = 0U;
  event_head = event_tail = event_count = 0U;
  wifi_state = WIFI_BACKOFF;
  wifi_since = now;
  bmc_tcp_reset_at_capture();
  if(tcp_callbacks.link_changed) tcp_callbacks.link_changed(0U);
}

/* Stage names deliberately omit AP credentials. */
static const char *bmc_tcp_stage(void)
{
  static const char *const stages[] = {"AT", "ATE0", "CWMODE", "CWJAP", "CIPMUX", "CIPSTART"};
  if(wifi_operation == WIFI_CLOSE) return "CIPCLOSE";
  if(wifi_operation == WIFI_SEND_PROMPT)
  {
    return "CIPSEND prompt";
  }
  if(wifi_operation == WIFI_SEND_ACK)
  {
    return "CIPSEND acknowledgement";
  }
  return wifi_step < 6U ? stages[wifi_step] : "TCP";
}

static void bmc_tcp_fail(uint32_t now, const char *reason)
{
  printf("[WIFI] %s: %s; retry in %lu ms\r\n", bmc_tcp_stage(), reason,
         (unsigned long)BMC81M001_WIFI_RECONNECT_MS);
  bmc_tcp_disconnect(now);
}

static uint8_t bmc_tcp_begin_bytes(const uint8_t *data, size_t length,
                                      bmc_tcp_operation_t operation,
                                      uint32_t timeout, uint32_t now)
{
  bmc_tcp_reset_at_capture();
  if(!bmc_tcp_tx_bytes(BMC81M001_USART_PORT, &wifi_tx, data, length))
  {
    bmc_tcp_fail(now, "TX unavailable");
    return 0U;
  }
  wifi_operation = operation;
  wifi_timeout = timeout;
  wifi_since = now;
  wifi_state = WIFI_TX;
  return 1U;
}

static uint8_t bmc_tcp_begin(const char *text, bmc_tcp_operation_t operation,
                                uint32_t timeout, uint32_t now)
{
  return bmc_tcp_begin_bytes((const uint8_t *)text, strlen(text), operation, timeout, now);
}

static void bmc_tcp_connect_step(uint32_t now)
{
  char cmd[WIFI_TX_MAX];
  uint32_t timeout = 3000U;
  int length = 0;
  switch(wifi_step)
  {
    case 0: length = snprintf(cmd, sizeof(cmd), "AT\r\n"); timeout = 2500U; break;
    case 1: length = snprintf(cmd, sizeof(cmd), "ATE0\r\n"); timeout = 2500U; break;
    case 2: length = snprintf(cmd, sizeof(cmd), "AT+CWMODE=1\r\n"); break;
    case 3:
      length = snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n",
                        BMC81M001_WIFI_SSID, BMC81M001_WIFI_PASSWORD);
      timeout = 20000U;
      break;
    case 4: length = snprintf(cmd, sizeof(cmd), "AT+CIPMUX=0\r\n"); break;
    case 5:
      length = snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u\r\n",
                        BMC81M001_SERVER_IP, (unsigned)BMC81M001_SERVER_PORT);
      timeout = 10000U;
      break;
    default:
      bmc_tcp_disconnect(now);
      return;
  }
  if(length <= 0 || length >= (int)sizeof(cmd))
  {
    bmc_tcp_disconnect(now);
    return;
  }
  wifi_operation = WIFI_CONNECT;
  if(wifi_step == 5U)
  {
    printf("[WIFI] CIPSTART TCP %s:%u\r\n", BMC81M001_SERVER_IP, (unsigned)BMC81M001_SERVER_PORT);
  }
  else
  {
    printf("[WIFI] %s\r\n", bmc_tcp_stage());
  }
  bmc_tcp_begin(cmd, WIFI_CONNECT, timeout, now);
}

static void bmc_tcp_tick(uint32_t now)
{
  char cmd[40];
  uint8_t success;
  if(wifi_state == WIFI_BACKOFF)
  {
    if((uint32_t)(now - wifi_since) < BMC81M001_WIFI_RECONNECT_MS)
    {
      return;
    }
    wifi_step = 0U;
    wifi_state = WIFI_START;
  }
  if(wifi_state == WIFI_START)
  {
    if(wifi_close_required)
      bmc_tcp_begin("AT+CIPCLOSE\r\n", WIFI_CLOSE, 3000U, now);
    else
      bmc_tcp_connect_step(now);
    return;
  }
  if(wifi_state == WIFI_TX)
  {
    if(bmc_tcp_tx_done(BMC81M001_USART_PORT, &wifi_tx))
    {
      wifi_state = WIFI_REPLY;
      wifi_since = now;
    }
    else
    {
      if(wifi_tx.failed || (uint32_t)(now - wifi_since) >= WIFI_TX_TIMEOUT_MS)
      {
        bmc_tcp_fail(now, "UART TX timeout");
      }
      return;
    }
  }
  if(wifi_state == WIFI_REPLY)
  {
    if(wifi_operation == WIFI_CLOSE)
    {
      /* ERROR is also returned if the previous socket is already closed. */
      if(bmc_tcp_result_line(wifi_at, "OK") || bmc_tcp_result_line(wifi_at, "ERROR") ||
         bmc_tcp_result_line(wifi_at, "CLOSED"))
      {
        wifi_close_required = 0U;
        wifi_state = WIFI_START;
        bmc_tcp_reset_at_capture();
      }
      else if((uint32_t)(now - wifi_since) >= wifi_timeout)
        bmc_tcp_fail(now, "close timeout");
      return;
    }
    success = 0U;
    /* ALREADY CONNECT may be followed by ERROR on some ESP-AT versions. */
    if(wifi_operation == WIFI_CONNECT && wifi_step == 5U &&
       bmc_tcp_result_line(wifi_at, "ALREADY CONNECT"))
    {
      success = 1U;
    }
    if(!success && (bmc_tcp_result_line(wifi_at, "ERROR") ||
                   bmc_tcp_result_line(wifi_at, "FAIL") ||
                   bmc_tcp_result_line(wifi_at, "SEND FAIL") || strstr(wifi_at, "busy p")))
    {
      const char *reason = strstr(wifi_at, "busy p") ? "module busy" :
                           bmc_tcp_result_line(wifi_at, "ERROR") ? "ERROR reply" : "FAIL reply";
      bmc_tcp_fail(now, reason);
      return;
    }
    if(wifi_operation == WIFI_CONNECT)
    {
      success = success || bmc_tcp_result_line(wifi_at, "OK");
    }
    else if(wifi_operation == WIFI_SEND_PROMPT)
    {
      success = strstr(wifi_at, ">") != NULL;
    }
    else
    {
      success = bmc_tcp_result_line(wifi_at, "SEND OK");
    }
    if(!success)
    {
      if((uint32_t)(now - wifi_since) >= wifi_timeout)
      {
        bmc_tcp_fail(now, "reply timeout");
      }
      return;
    }
    if(wifi_operation == WIFI_CONNECT)
    {
      wifi_step++;
      if(wifi_step == 6U)
      {
        wifi_connected = 1U;
        printf("[WIFI] TCP connected %s:%u\r\n", BMC81M001_SERVER_IP, (unsigned)BMC81M001_SERVER_PORT);
        wifi_state = WIFI_ONLINE;
        if(tcp_callbacks.link_changed) tcp_callbacks.link_changed(1U);
      }
      else
      {
        wifi_state = WIFI_START;
      }
      bmc_tcp_reset_at_capture();
    }
    else if(wifi_operation == WIFI_SEND_PROMPT)
    {
      bmc_tcp_begin_bytes(bmc_tcp_send_data() + send_offset, send_chunk,
                             WIFI_SEND_ACK, 5000U, now);
    }
    else
    {
      send_offset = (uint16_t)(send_offset + send_chunk);
      if(send_offset >= bmc_tcp_send_length())
      {
        if(send_kind == SEND_PACKET)
        {
          uint32_t completed_token = packets[packet_head].token;
          packet_head = (uint8_t)((packet_head + 1U) % BMC81M001_PACKET_COUNT);
          --packet_count;
          if(tcp_callbacks.packet_sent) tcp_callbacks.packet_sent(completed_token);
        }
        else
        {
          event_head = (uint8_t)((event_head + 1U) % EVENT_COUNT);
          --event_count;
        }
        send_kind = SEND_NONE;
        send_offset = 0U;
      }
      wifi_state = WIFI_ONLINE;
      bmc_tcp_reset_at_capture();
    }
    return;
  }
  if(wifi_state == WIFI_ONLINE)
  {
    if(send_kind == SEND_NONE)
    {
      /* Short records drain between complete retained packets. */
      if(event_count != 0U) send_kind = SEND_CONTROL;
      else if(packet_count != 0U) send_kind = SEND_PACKET;
      else return;
      send_offset = 0U;
    }
    uint16_t remaining = (uint16_t)(bmc_tcp_send_length() - send_offset);
    send_chunk = remaining > WIFI_CHUNK_SIZE ? WIFI_CHUNK_SIZE : remaining;
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u\r\n", (unsigned)send_chunk);
    bmc_tcp_begin(cmd, WIFI_SEND_PROMPT, 4000U, now);
  }
}

void BMC81M001_TcpInit(const BMC81M001_TcpCallbacks *callbacks)
{
  bmc_tcp_dma_init();
  memset(&tcp_callbacks, 0, sizeof(tcp_callbacks));
  if(callbacks) tcp_callbacks = *callbacks;
  packet_head = packet_tail = packet_count = 0U;
  send_kind = SEND_NONE;
  send_offset = send_chunk = 0U;
  wifi_close_required = 0U;
  wifi_connected = 0U;
  wifi_step = 0U;
  wifi_state = WIFI_START;
  event_head = event_tail = event_count = 0U;
  ipd_mode = ipd_hn = ipd_match = 0U;
  ipd_remaining = 0U;
  bmc_tcp_reset_at_capture();
  bmc_tcp_tx_cancel(BMC81M001_USART_PORT, &wifi_tx);
}

void BMC81M001_Process(void)
{
  uint32_t now = SysTick_GetTick();
  bmc_tcp_poll();
  /* CWMODE/CWJAP can disconnect an old AP before CONNECTED/GOT IP/OK.
     During AP setup, let the command's final reply or timeout decide success.
     After CWJAP succeeds, link loss must still cancel TCP/measurement work. */
  if((wifi_connected || (wifi_step >= 4U && wifi_state != WIFI_BACKOFF)) &&
     (bmc_tcp_result_line(wifi_at, "CLOSED") ||
      bmc_tcp_result_line(wifi_at, "WIFI DISCONNECT")))
  {
    bmc_tcp_fail(now, "connection lost");
  }
  bmc_tcp_tick(now);
}

uint8_t BMC81M001_IsConnected(void) { return wifi_connected; }
// END BMC NONBLOCKING TCP
