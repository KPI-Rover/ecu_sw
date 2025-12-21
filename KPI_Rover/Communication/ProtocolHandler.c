#include "ProtocolHandler.h"
#include "cmsis_os.h"
#include "UARTTransport.h"

//osMessageQueueId_t messageQueue;

void ProtocolHandler_run() {
	uint8_t msg_ptr;
	osMessageQueueGet(messageQueue, &msg_ptr, NULL, 0);
	ProtocolHandler_processRequest(&msg_ptr);
}

void ProtocolHandler_processRequest(uint8_t *request){
	enum COMMAND_ID command_id = request[0];
	uint8_t *payload = request + 1;
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

