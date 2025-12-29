#include "UARTTransport.h"
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

	requestQueue = osMessageQueueNew(10, 10, NULL);
	if (!requestQueue) {
		return false;
	}

	answerQueue = osMessageQueueNew(10, 10, NULL);
	if (!answerQueue) {
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
	uint8_t *answer_ptr = malloc(10);
	osStatus_t status;
	while (true) {
		status = osMessageQueueGet(answerQueue, answer_ptr, NULL, 0);
		if (status == osOK) {
			UARTTransport_send(answer_ptr, 10);
		}
	}
}

void UARTTransport_onUartReceive(const unsigned char *msg_ptr, short unsigned int length) {
	uint8_t frame_length = msg_ptr[0];

	//	if crc16 is broken, return or do something
	if (crc16(msg_ptr, length)) {
		return;
	}

	osMessageQueuePut(requestQueue, msg_ptr + 1, 0, 0);
}
