#ifndef COMMUNICATION_UARTTRANSPORT_H_
#define COMMUNICATION_UARTTRANSPORT_H_

#include <stdint.h>

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef UARTTransport_init(void);
void UARTTransport_send(uint8_t *data, uint16_t length);
//void UARTTransport_receive(void);
void UARTTransport_run(void *arg);

void UARTTransport_onUartReceive(UART_HandleTypeDef *huart, uint16_t Size);

#endif /* COMMUNICATION_UARTTRANSPORT_H_ */
