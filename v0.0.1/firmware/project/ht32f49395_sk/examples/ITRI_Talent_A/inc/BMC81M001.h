/*********************************************************************************************************//**
 * @file    BMC81M001_HT32/src/BMC81M001.h
 * @version V1.0.1
 * @date    2024-10-10
 * @brief   The header file of BMC81M001 driver.
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
/* Define to prevent recursive inclusion -------------------------------------------------------------------*/
#ifndef _BMC81M001_H
#define _BMC81M001_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------------------*/
#include "ht32.h"
#include "string.h"
#include "ht32f493x5_usart.h"
#include "i2c_application.h"

#include <stdbool.h>
#include <stdint.h>

/* Settings ------------------------------------------------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------------------------------------*/
typedef enum {
  BMC81M001_SUCCESS,
  BMC81M001_FAILURE
} BMC81M001_selStatus;
/* Exported constants --------------------------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------------------------------------*/

#define BMC81M001_UART_TXTIMEOUT          1000   //uint:baud clock
#define BMC81M001_UART_RXTIMEOUT_BYTE     10      //uint:ms

#define BMC81M001_baudRate 115200
#define BMC81M001_SEND_SUCCESS 1
#define BMC81M001_SEND_FAIL 0

#define BMC81M001_USART_PORT             UART4
#define BMC81M001_USART_CLK              CRM_UART4_PERIPH_CLOCK

#define BMC81M001_USART_TX_PIN           GPIO_PINS_10
#define BMC81M001_USART_TX_GPIO_PORT     GPIOC
#define BMC81M001_USART_TX_GPIO_CLK      CRM_GPIOC_PERIPH_CLOCK

#define BMC81M001_USART_RX_PIN           GPIO_PINS_11
#define BMC81M001_USART_RX_GPIO_PORT     GPIOC
#define BMC81M001_USART_RX_GPIO_CLK      CRM_GPIOC_PERIPH_CLOCK

#define BMC81M001_USART_IRQn             UART4_IRQn
#define BMC81M001_RX_BUFFER_SIZE         2048U

// BEGIN BMC TCP API
#ifndef BMC81M001_WIFI_SSID
#define BMC81M001_WIFI_SSID "Ying"
#endif
#ifndef BMC81M001_WIFI_PASSWORD
#define BMC81M001_WIFI_PASSWORD "aaa12346"
#endif
#ifndef BMC81M001_SERVER_IP
#define BMC81M001_SERVER_IP "172.20.10.4"
#endif
#ifndef BMC81M001_SERVER_PORT
#define BMC81M001_SERVER_PORT 5000U
#endif


#define BMC81M001_WIFI_RECONNECT_MS 3000U

#define BMC81M001_PACKET_MAX 9344U
#define BMC81M001_PACKET_COUNT 2U
#define BMC81M001_CONTROL_MAX 200U
#define BMC81M001_CONTROL_COUNT 8U

typedef struct {
  const uint8_t *data;
  uint16_t length;
} BMC81M001_DataSpan;
typedef struct {
  void (*receive_byte)(uint8_t data);
  void (*link_changed)(uint8_t connected);
  void (*packet_sent)(uint32_t token);
} BMC81M001_TcpCallbacks;

/* Call after UART init. Callbacks run in main context and must not block.
 * Packet-sent means module SEND OK, not PC application acknowledgement.
 * Do not mix this service with the legacy blocking AT/TCP/MQTT API. */
void BMC81M001_TcpInit(const BMC81M001_TcpCallbacks *callbacks);
void BMC81M001_Process(void);
uint8_t BMC81M001_IsConnected(void);
uint8_t BMC81M001_ControlSlotsFree(void);
/* Short records: copy now, require connection, discard on disconnect. */
uint8_t BMC81M001_QueueData(const uint8_t *data, uint16_t length);
/* Copy spans into ONE atomic record; survive reconnect, not reset.
 * Return 0 for invalid/full; no partial enqueue. token is opaque to driver. */
uint8_t BMC81M001_QueuePacket(const BMC81M001_DataSpan *parts, uint8_t count, uint32_t token);
// END BMC TCP API

#define RES_MAX_LENGTH 200
/* Exported variables --------------------------------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------------------------------------*/
BMC81M001_selStatus BMC81M001_selSerial(usart_type *serial_port);
void BMC81M001_Init(uint32_t BaudRate); 
bool BMC81M001_connectToAP(char ssid[],char pass[]);
bool BMC81M001_connectTCP(char ip[],int port);
bool BMC81M001_writeDataTcp(int Dlength,char Dbuffer[]);
bool BMC81M001_readDataTcp(char readbuff[]);
bool BMC81M001_configMqtt(char clientid[],char username[],char password[],char mqtt_host[],int server_port);
bool BMC81M001_setPublishTopic(char publishtopic[]);
bool BMC81M001_setSubscribetopic(char subscribetopic[]);
bool BMC81M001_setTopic(char topic[]);
bool BMC81M001_writeByte(char Dbuffer[],int Dlength,char topic[]);
void BMC81M001_readIotData(char IotReciveBuff[],int *IotReciveBufflen,char IotReciveTopic[]);
bool BMC81M001_reset(void);
void BMC81M001_writeBytes(char wbuf[]);
void BMC81M001_readBytes(uint8_t rbuf[],int timeout, uint8_t reTry);
void BMC81M001_RxIrqHandler(void);
/* Single-consumer raw RX API; do not mix with AT/readDataTcp during nonblocking TCP service. */
bool BMC81M001_readByte(uint8_t *data);
int BMC81M001_sendATCommand(char StringstrCmd[],uint8_t reTry);
void BMC81M001_arrayAdd(char array1[],char array2[]);
void _BMC81M001_clearBuf(void);
void _BMC81M001_delay(vu32 count);
void BMC81M001_clearResponse(char Dbuffer[]);
int BMC81M001_arrayFind(char array1[],char array2);
#ifdef __cplusplus
}
#endif

#endif

