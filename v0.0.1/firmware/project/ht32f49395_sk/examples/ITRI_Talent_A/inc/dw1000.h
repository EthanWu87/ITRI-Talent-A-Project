#ifndef _DW1000_H_
#define _DW1000_H_

#include "ht32f493x5_board.h"
#include <stdint.h>

/* Override for each board. Network settings originate from the supplied example. */
#ifndef DW1000_NODE_ID
#define DW1000_NODE_ID 'A'
#endif
#ifndef DW1000_WIFI_SSID
#define DW1000_WIFI_SSID "Ying"
#endif
#ifndef DW1000_WIFI_PASSWORD
#define DW1000_WIFI_PASSWORD "aaa12346"
#endif
#ifndef DW1000_SERVER_IP
#define DW1000_SERVER_IP "172.20.10.3"
#endif
#ifndef DW1000_SERVER_PORT
#define DW1000_SERVER_PORT 5000U
#endif

/* DW1000 USART configuration */
#define DW1000_USART_PORT             UART8
#define DW1000_USART_CLK              CRM_UART8_PERIPH_CLOCK

#define DW1000_USART_TX_PIN           GPIO_PINS_1
#define DW1000_USART_TX_GPIO_PORT     GPIOE
#define DW1000_USART_TX_GPIO_CLK      CRM_GPIOE_PERIPH_CLOCK

#define DW1000_USART_RX_PIN           GPIO_PINS_0
#define DW1000_USART_RX_GPIO_PORT     GPIOE
#define DW1000_USART_RX_GPIO_CLK      CRM_GPIOE_PERIPH_CLOCK

#define DW1000_USART_IRQn             UART8_IRQn
#define DW1000_BAUD_RATE 115200U
#define DW1000_RX_BUFFER_SIZE 2048U
#define DW1000_RECONNECT_MS 3000U

#ifdef __cplusplus
extern "C" {
#endif

void dw1000_init(void);
void dw1000_receive_byte(uint8_t data);
/* Bounded nonblocking service: call in the main loop and after inference. */
void dw1000_process(void);
/* Called by UART4/UART8 handlers to send the next queued byte. */
void dw1000_tx_irq_handler(usart_type *port);

#ifdef __cplusplus
}
#endif
#endif
