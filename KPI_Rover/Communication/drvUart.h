#ifndef COMMUNICATION_DRVUART_H_
#define COMMUNICATION_DRVUART_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef void (*drvUart_OnReceiveCallback)(const unsigned char *, short unsigned int);

bool drvUart_init(void);
HAL_StatusTypeDef drvUart_send(uint8_t *data, uint16_t length);
void drvUart_registerCallback(drvUart_OnReceiveCallback onReceive);

#endif /* COMMUNICATION_DRVUART_H_ */
