#include "ProtocolHandler.h"
#include "cmsis_os.h"
#include "UARTTransport.h"
#include "messageQueueId.h"
#include <stdlib.h>

void ProtocolHandler_run() {
	uint8_t *msg_ptr = malloc(10);
	uint8_t *answer_ptr = malloc(10);

	osStatus_t status;
	while (true) {
		status = osMessageQueueGet(requestQueue, msg_ptr, NULL, 0);
		if (status == osOK) {
			ProtocolHandler_processRequest(msg_ptr, answer_ptr);
			osMessageQueuePut(answerQueue, answer_ptr, 0, 0);
		}
	}
}

void ProtocolHandler_processRequest(uint8_t *request, uint8_t *response){
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

	*response = 0;
}

