#include "drvUart.h"
#include "ulog.h"

UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
HAL_StatusTypeDef drvUart_init(void) {
	/**
	  * Enable DMA controller clock
	  */
	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	/* DMA1_Stream3_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_StatusTypeDef status = HAL_UART_Init(&huart3);
	if (status != HAL_OK) {
		ULOG_ERROR("UART initialization error!");
	}

	return status;
}

HAL_StatusTypeDef drvUart_send(uint8_t *data, uint16_t length) {
	return HAL_UART_Transmit_DMA(&huart3, data, length);
}

HAL_StatusTypeDef drvUart_registerCallback(pUART_CallbackTypeDef onReceive) {
  return HAL_UART_RegisterRxEventCallback(&huart3, onReceive);
}
