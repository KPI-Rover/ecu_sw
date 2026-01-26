#ifndef COMMUNICATION_DRVUART_H_
#define COMMUNICATION_DRVUART_H_

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef drvUart_init(void);
HAL_StatusTypeDef drvUart_send(uint8_t *data, uint16_t length);
HAL_StatusTypeDef drvUart_registerCallback(pUART_RxEventCallbackTypeDef onReceive);

#endif /* COMMUNICATION_DRVUART_H_ */
