#ifndef __DRV_UART_H
#define __DRV_UART_H

#include <stdbool.h>

#define DRV_UART_RECEIVE_BUFFER_SIZE 256
#define DRV_UART_TRANSMIT_BUFFER_SIZE 255

bool drvUart_start(void);
bool drvUart_set_on_rx_cplt(void (*f)(const uint8_t * const buffer));
bool drvUart_set_on_tx_cplt(void (*f)(void));
bool drvUart_send(uint8_t *buf);


#endif /* __DRV_UART_H */
