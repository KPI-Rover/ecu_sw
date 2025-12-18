#include "UARTTransport.h"
#include "drvUart.h"
#include "cmsis_os.h"


osMessageQueueId_t messageQueue;

bool UARTTransport_init(void) {
	if (!drvUart_init()) {
		return false;
	}

	drvUart_registerCallback(onUartReceive);

	messageQueue = osMessageQueueNew(10, 10, NULL);
	if (!messageQueue) {
		return false;
	}

	return true;
}

void UARTTransport_send(uint8_t *data, uint16_t length) {
	drvUart_send(data, length);
}

void UARTTransport_receive(void) {
	uint8_t *msg_ptr;
	osMessageQueueGet(messageQueue, &msg_ptr, NULL, 0);

	uint8_t frame_length = msg_ptr[0];
	uint16_t crc16 = msg_ptr[frame_length - 1];
	(void) crc16;
//	if crc16 is broken, return or do something

	enum COMMAND_ID command_id = msg_ptr[1];
	uint8_t *payload = msg_ptr + 2;
	(void) payload;

	switch (command_id) {
		case GET_API_VERSION:
			break;
		case SET_MOTOR_SPEED:
			break;
		case SET_ALL_MOTORS_SPEED:
			break;
		case GET_ENCODER:
			break;
		case GET_ALL_ENCODERS:
			break;
		case CONNECT_UDP:
			break;
		default:
			break;
	}
}

void UARTTransport_run(void) {

}

void onUartReceive(const unsigned char *, short unsigned int) {

}
