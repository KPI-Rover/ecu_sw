#include "UARTTransport.h"

#include <string.h>
#include <stdint.h>
#include "cmsis_os2.h"

#define SEND_BUFFER_SIZE 64
#define RECV_BUFFER_SIZE 32

static uint8_t sendBuffer[SEND_BUFFER_SIZE];
static uint8_t recvBuffer[RECV_BUFFER_SIZE];
static uint8_t recvSize;

static void (*dispatch_table[256])(void);

static osThreadAttr_t ta = {
	.stack_size = 64 * 4
};

static void dispatch_unsupported(void)
{
	sendBuffer[0] = 0xFF;
	UARTTransport_send(sendBuffer, 1);
}

static void dispatch_01(void)
{
	sendBuffer[0] = 0x01;
	sendBuffer[1] = 0x01;
	UARTTransport_send(sendBuffer, 2);
}

static void dispatch_02(void)
{
	sendBuffer[0] = 0x02;

	switch (recvBuffer[1]) {
	case 0:
		sendBuffer[1] = 0x0;
		break;
	case 1:
		sendBuffer[1] = 0x0;
		break;
	case 2:
		sendBuffer[1] = 0x0;
		break;
	case 3:
		sendBuffer[1] = 0x0;
		break;
	default:
		sendBuffer[1] = 0x01;
		break;
	}

	UARTTransport_send(sendBuffer, 2);
}

static void dispatch_03(void)
{
	sendBuffer[0] = 0x03;
	sendBuffer[1] = 0x00;
	UARTTransport_send(sendBuffer, 2);
}

static void dispatch_05(void)
{
	sendBuffer[0] = 0x05;
	memset(sendBuffer + 1, 0xBB, 16);
	UARTTransport_send(sendBuffer, 17);
}

static void dispatch_06(void)
{
	sendBuffer[0] = 0x06;
	memset(sendBuffer + 1, 0xCC, 52);
	UARTTransport_send(sendBuffer, 53);
}

void ProtocolHandler_processTask(void *d)
{
	(void) d;

	UARTTransport_init();

	for ( ; ; ) {
		UARTTransport_receive(recvBuffer, &recvSize);
		dispatch_table[recvBuffer[0]]();
	}
}

void ProtocolHandler_init(void)
{
	for (uint32_t i = 0; i < 256; i++) {
		dispatch_table[i] = dispatch_unsupported;
	}

	dispatch_table[1] = dispatch_01;
	dispatch_table[2] = dispatch_02;
	dispatch_table[3] = dispatch_03;
	dispatch_table[5] = dispatch_05;
	dispatch_table[6] = dispatch_06;

	(void) osThreadNew(ProtocolHandler_processTask, NULL, &ta);
}
