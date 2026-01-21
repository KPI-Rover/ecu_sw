#ifndef COMMUNICATION_UARTTRANSPORT_H_
#define COMMUNICATION_UARTTRANSPORT_H_

#include <stdint.h>
#include <stdbool.h>

bool UARTTransport_init(void);
void UARTTransport_send(uint8_t *data, uint16_t length);
//void UARTTransport_receive(void);
void UARTTransport_run(void *arg);

void UARTTransport_onUartReceive(const unsigned char *, short unsigned int);

#endif /* COMMUNICATION_UARTTRANSPORT_H_ */
