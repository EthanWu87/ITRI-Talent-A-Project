#ifndef _DW1000_H_
#define _DW1000_H_

#include "ht32f493x5_board.h"
#include <stdint.h>

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

#ifdef __cplusplus
extern "C" {
#endif


/* Commands are dispatched by the application's TCP service. */
void dw1000_execute_command(const char *line);
void dw1000_on_disconnect(void);
uint8_t dw1000_is_busy(void);

void dw1000_init(void);
void dw1000_receive_byte(uint8_t data);
/* Bounded nonblocking service: call in the main loop and after inference. */
void dw1000_process(void);
/* Called by UART8 handler to send the next queued byte. */
void dw1000_tx_irq_handler(usart_type *port);

#ifdef __cplusplus
}
#endif
#endif
