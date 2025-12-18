#include "drvUart.h"

UART_HandleTypeDef huart;
DMA_HandleTypeDef hdma_usart3_rx;

bool drvUart_init(void)
{
	huart.Instance = USART3;
	huart.Init.BaudRate = 115200;
	huart.Init.WordLength = UART_WORDLENGTH_8B;
	huart.Init.StopBits = UART_STOPBITS_1;
	huart.Init.Parity = UART_PARITY_NONE;
	huart.Init.Mode = UART_MODE_TX_RX;
	huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart) != HAL_OK)
	{
		return false;
	}

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

	return true;
}

HAL_StatusTypeDef drvUart_send(uint8_t *data, uint16_t length)
{
	return HAL_UART_Transmit_DMA(&huart, data, length);
}

HAL_StatusTypeDef registerCallback(drvUart_OnReceiveCallback onReceive)
{
	return HAL_UART_RegisterRxEventCallback(&huart, onReceive);
}
