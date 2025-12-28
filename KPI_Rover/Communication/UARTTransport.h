#ifndef COMMUNICATION_UARTTRANSPORT_H_
#define COMMUNICATION_UARTTRANSPORT_H_

#include <stdint.h>
#include <stdbool.h>

enum COMMAND_ID {
	GET_API_VERSION = 0x01,
	SET_MOTOR_SPEED = 0X02,
	SET_ALL_MOTORS_SPEED = 0X03,
	GET_ENCODER = 0X04,
	GET_ALL_ENCODERS = 0X05,
	CONNECT_UDP = 0X06,
};


bool UARTTransport_init(void);
void UARTTransport_send(uint8_t *data, uint16_t length);
//void UARTTransport_receive(void);
void UARTTransport_run(void *arg);

void UARTTransport_onUartReceive(const unsigned char *, short unsigned int);

#endif /* COMMUNICATION_UARTTRANSPORT_H_ */
