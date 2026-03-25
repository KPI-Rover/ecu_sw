#include <string.h>

#include "UARTTransport.h"
#include "cmsis_os2.h"

#include "drvUart.h"

#define FLAG_TRANSMIT_FINISH 0x1

static osMessageQueueId_t sendQ;
static osMessageQueueId_t recvQ;

static osEventFlagsId_t ef;

static osThreadAttr_t ta = {
	.stack_size = 64 * 4
};

void UARTTransport_sendTask(void *d)
{
	(void) d;

	static uint8_t sendMessageBuffer[UART_TRANSPORT_SEND_BUFFER_SIZE];

	for ( ; ; ) {
		if (osMessageQueueGet(sendQ, sendMessageBuffer, NULL, osWaitForever))
			continue;

		drvUart_send(sendMessageBuffer);

		osEventFlagsWait(ef, FLAG_TRANSMIT_FINISH, 0, osWaitForever);
	}
}

void UARTTransport_onRxCplt(const uint8_t * const buffer)
{
	(void) osMessageQueuePut(recvQ, buffer, 0, 0);
}

void UARTTransport_onTxCplt(void)
{
	(void) osEventFlagsSet(ef, FLAG_TRANSMIT_FINISH);
}

void UARTTransport_init(void)
{
	(void) drvUart_set_on_rx_cplt(UARTTransport_onRxCplt);
	(void) drvUart_set_on_tx_cplt(UARTTransport_onTxCplt);

	(void) drvUart_start();

	ef = osEventFlagsNew(NULL);

	sendQ = osMessageQueueNew(16, UART_TRANSPORT_SEND_BUFFER_SIZE, NULL);
	recvQ = osMessageQueueNew(16, UART_TRANSPORT_RECV_BUFFER_SIZE, NULL);

	(void) osThreadNew(UARTTransport_sendTask, NULL, &ta);
}

void UARTTransport_receive(uint8_t * const buf, uint8_t * const size)
{
	static uint8_t recvBuffer[UART_TRANSPORT_RECV_BUFFER_SIZE];

	(void) osMessageQueueGet(recvQ, recvBuffer, NULL, osWaitForever);

	*size = recvBuffer[0] - 1;
	memcpy(buf, recvBuffer + 1, *size);
}

void UARTTransport_send(const uint8_t * const buf, const uint8_t size)
{
	static uint8_t sendEncodingBuffer[UART_TRANSPORT_SEND_BUFFER_SIZE];

	sendEncodingBuffer[0] = size + 1;
	memcpy(sendEncodingBuffer + 1, buf, size);

	(void) osMessageQueuePut(sendQ, sendEncodingBuffer, 0, osWaitForever);
}
