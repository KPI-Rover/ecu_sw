#include "main.h"
#include <stdbool.h>
#include <string.h>

#include "drvUart.h"

#include "crc16.h"

#include "cmsis_os2.h"

extern UART_HandleTypeDef huart3;

static uint8_t *activeReceiveBuffer;
static uint8_t *sideReceiveBuffer;

static uint8_t receiveBuffer1[DRV_UART_RECEIVE_BUFFER_SIZE];
static uint8_t receiveBuffer2[DRV_UART_RECEIVE_BUFFER_SIZE];
static uint8_t transmitBuffer[DRV_UART_TRANSMIT_BUFFER_SIZE];

static void (*on_rx_cplt)(const uint8_t * const buffer);
static void (*on_tx_cplt)(void);

DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;


static void drvUart_configureDMA(void)
{
	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	/* DMA1_Stream3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

	/* USART3 DMA Init */
	/* USART3_RX Init */
	hdma_usart3_rx.Instance = DMA1_Stream1;
	hdma_usart3_rx.Init.Channel = DMA_CHANNEL_4;
	hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_usart3_rx.Init.Mode = DMA_NORMAL;
	hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma_usart3_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma_usart3_rx.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma_usart3_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
	if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
	{
		Error_Handler();
	}

	__HAL_LINKDMA(&huart3, hdmarx, hdma_usart3_rx);

	/* USART3_TX Init */
	hdma_usart3_tx.Instance = DMA1_Stream3;
	hdma_usart3_tx.Init.Channel = DMA_CHANNEL_4;
	hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_usart3_tx.Init.Mode = DMA_NORMAL;
	hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma_usart3_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma_usart3_tx.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma_usart3_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
	if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
	{
		Error_Handler();
	}

	__HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx);

	/* USART3 interrupt Init */
	HAL_NVIC_SetPriority(USART3_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(USART3_IRQn);
}

bool drvUart_start(void)
{
	activeReceiveBuffer = receiveBuffer1;
	sideReceiveBuffer = receiveBuffer2;

	transmitBuffer[0] = 0xAA;

	drvUart_configureDMA();

	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, activeReceiveBuffer, DRV_UART_RECEIVE_BUFFER_SIZE);

	return true;
}

bool drvUart_on_rx_cplt(void (*f)(const uint8_t * const buffer))
{
	on_rx_cplt = f;

	return true;
}

bool drvUart_on_tx_cplt(void (*f)(void))
{
	on_tx_cplt = f;

	return true;
}

bool drvUart_send(uint8_t * const buf)
{
	memcpy(transmitBuffer + 1, buf, buf[0]);
	transmitBuffer[1] += 2; // add size of CRC16

	uint16_t crc = crc16(transmitBuffer + 1, buf[0]);

	transmitBuffer[1 + buf[0]] = (uint8_t) crc;
	transmitBuffer[1 + buf[0] + 1] = (uint8_t) (crc >> 8);

	HAL_UART_Transmit_DMA(&huart3, transmitBuffer, buf[0] + 3);

	return true;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	// first, relaunch hardware DMA reception on the other buffer
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, sideReceiveBuffer, DRV_UART_RECEIVE_BUFFER_SIZE);

	// then update local references
	{
		// restrict symbol visibility, but use static allocation for performance
		static uint8_t *tmp;

		tmp = sideReceiveBuffer;
		sideReceiveBuffer = activeReceiveBuffer;
		activeReceiveBuffer = tmp;
	}

	if (on_rx_cplt == NULL)
		return;

	// then process every received packet
	{
		static uint16_t offset;

		for (offset = 0; offset < Size; offset += 3 + sideReceiveBuffer[1 + offset]) {
			// verify packet is not corrupted
			if (crc16(sideReceiveBuffer + 1 + offset, sideReceiveBuffer[1 + offset])) {
				// drop packet on failure
				return;
			}

			sideReceiveBuffer[1 + offset] -= 2; // exclude size of CRC16

			// then call the callback (stripping 0xAA from the data)
			on_rx_cplt(sideReceiveBuffer + 1 + offset);
		}
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (on_tx_cplt == NULL)
		return;

	on_tx_cplt();
}
