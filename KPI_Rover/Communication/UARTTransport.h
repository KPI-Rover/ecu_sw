#ifndef __UART_TRANSPORT_H
#define __UART_TRANSPORT_H

#include <stdint.h>

#define UART_TRANSPORT_RECV_BUFFER_SIZE 32
#define UART_TRANSPORT_SEND_BUFFER_SIZE 64

void UARTTransport_init(void);
void UARTTransport_receive(uint8_t * const buf, uint8_t * const size);
void UARTTransport_send(const uint8_t * const buf, const uint8_t size);

#endif /* __UART_TRANSPORT_H */
