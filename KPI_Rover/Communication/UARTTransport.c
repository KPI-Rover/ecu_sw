#include <stdlib.h>
#include <stdbool.h>

#include "UARTTransport.h"
#include "ProtocolHandler.h"
#include "drvUart.h"
#include "cmsis_os2.h"
#include "messageQueueId.h"
#include "crc16.h"
#include "ulog.h"

osMessageQueueId_t requestQueue;
osMessageQueueId_t responseQueue;
extern uint8_t UART_rx_buffer[56];

static const osThreadAttr_t UARTTransport_attributes = { .name =
		"UARTTransport_run", .stack_size = 128 * 4, .priority =
		(osPriority_t) osPriorityNormal, };

HAL_StatusTypeDef UARTTransport_init(void) {
	HAL_StatusTypeDef status = drvUart_init();
	if (status != HAL_OK) {
		return status;
	}

	drvUart_registerCallback(UARTTransport_onUartReceive);

	requestQueue = osMessageQueueNew(32, 17, NULL);
	if (!requestQueue) {
		return false;
	}

	responseQueue = osMessageQueueNew(32, 53, NULL);
	if (!responseQueue) {
		return false;
	}

	osThreadNew(UARTTransport_run, NULL, &UARTTransport_attributes);

	return HAL_OK;
}

void UARTTransport_send(uint8_t *data, uint16_t length) {
	drvUart_send(data, length);
}

void UARTTransport_run(void *arg) {
	uint8_t response_ptr[56];
	osStatus_t status;
	while (true) {
		status = osMessageQueueGet(responseQueue, response_ptr + 1, NULL, 0);
		if (status == osOK) {
			switch (response_ptr[1]) {
			case GET_API_VERSION:
				response_ptr[0] = sizeof(GET_API_VERSION_Response);
				break;
			case SET_MOTOR_SPEED:
				response_ptr[0] = sizeof(SET_MOTOR_SPEED_Response);
				break;
			case SET_ALL_MOTORS_SPEED:
				response_ptr[0] = sizeof(SET_ALL_MOTORS_SPEED_Response);
				break;
			case GET_ENCODER:
				response_ptr[0] = sizeof(GET_ENCODER_Response);
				break;
			case GET_ALL_ENCODERS:
				response_ptr[0] = sizeof(GET_ALL_ENCODERS_Response);
				break;
			case GET_IMU:
				response_ptr[0] = sizeof(GET_IMU_Response);
				break;
			default:
				ULOG_WARNING("Unknown command id: %d", response_ptr[1]);
				return;
			}

			response_ptr[0] += 4;

			uint16_t crc = crc16(response_ptr, response_ptr[0] - 2);
			response_ptr[response_ptr[0] - 2] = crc >> 8;
			response_ptr[response_ptr[0] - 1] = crc & 0xFF;

			UARTTransport_send(response_ptr, response_ptr[0]);
		}
	}
}

void UARTTransport_onUartReceive(UART_HandleTypeDef *huart, uint16_t Size) {
	//	if crc16 is broken, return or do something
	if (crc16(UART_rx_buffer + 1, UART_rx_buffer[0] - 1)) {
		return;
	}

	osMessageQueuePut(requestQueue, UART_rx_buffer + 1, 0, 0);
}
