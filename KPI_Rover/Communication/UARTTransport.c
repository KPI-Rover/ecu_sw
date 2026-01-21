#include "UARTTransport.h"
#include "ProtocolHandler.h"
#include "drvUart.h"
#include "cmsis_os.h"
#include "messageQueueId.h"
#include <stdlib.h>
#include "crc16.h"

bool UARTTransport_init(void) {
	if (!drvUart_init()) {
		return false;
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

	osThreadNew(UARTTransport_run, NULL, &(osThreadAttr_t){
			  .name = "UARTTransport_run",
			  .stack_size = 128 * 4,
			  .priority = (osPriority_t) osPriorityNormal,
			});

	return true;
}

void UARTTransport_send(uint8_t *data, uint16_t length) {
	drvUart_send(data, length);
}

//void UARTTransport_receive(void) {
//}

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
					break;
			}

			response_ptr[0] += 4;

			uint16_t crc =  crc16(response_ptr, response_ptr[0] - 2);
			response_ptr[response_ptr[0] - 2] = crc >> 8;
			response_ptr[response_ptr[0] - 1] = crc & 0xFF;

			UARTTransport_send(response_ptr, response_ptr[0]);
		}
	}
}

void UARTTransport_onUartReceive(const unsigned char *msg_ptr, short unsigned int length) {
	//	if crc16 is broken, return or do something
	if (crc16(msg_ptr + 1, length)) {
		return;
	}

	osMessageQueuePut(requestQueue, msg_ptr + 1, 0, 0);
}
