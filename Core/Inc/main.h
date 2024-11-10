/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define R_Pin GPIO_PIN_2
#define R_GPIO_Port GPIOE
#define G_Pin GPIO_PIN_3
#define G_GPIO_Port GPIOE
#define B_Pin GPIO_PIN_4
#define B_GPIO_Port GPIOE
#define UART2_TX_B_Pin GPIO_PIN_2
#define UART2_TX_B_GPIO_Port GPIOA
#define UART2_RX_B_Pin GPIO_PIN_3
#define UART2_RX_B_GPIO_Port GPIOA
#define SPI1_CS_IMU_Pin GPIO_PIN_4
#define SPI1_CS_IMU_GPIO_Port GPIOA
#define SPI1_CLK_IMU_Pin GPIO_PIN_5
#define SPI1_CLK_IMU_GPIO_Port GPIOA
#define SPI1_MISO_IMU_Pin GPIO_PIN_6
#define SPI1_MISO_IMU_GPIO_Port GPIOA
#define SPI1_MOSI_IMU_Pin GPIO_PIN_7
#define SPI1_MOSI_IMU_GPIO_Port GPIOA
#define F1_Pin GPIO_PIN_9
#define F1_GPIO_Port GPIOE
#define R1_Pin GPIO_PIN_10
#define R1_GPIO_Port GPIOE
#define TIM1_CH1_PWM1_Pin GPIO_PIN_11
#define TIM1_CH1_PWM1_GPIO_Port GPIOE
#define TIM1_CH3_PWM3_Pin GPIO_PIN_13
#define TIM1_CH3_PWM3_GPIO_Port GPIOE
#define TIM1_CH4_PWM4_Pin GPIO_PIN_14
#define TIM1_CH4_PWM4_GPIO_Port GPIOE
#define BUZZER_Pin GPIO_PIN_10
#define BUZZER_GPIO_Port GPIOD
#define R4_Pin GPIO_PIN_11
#define R4_GPIO_Port GPIOD
#define F4_Pin GPIO_PIN_12
#define F4_GPIO_Port GPIOD
#define R3_Pin GPIO_PIN_14
#define R3_GPIO_Port GPIOD
#define F3_Pin GPIO_PIN_15
#define F3_GPIO_Port GPIOD
#define TIM1_CH1_PWM1A8_Pin GPIO_PIN_8
#define TIM1_CH1_PWM1A8_GPIO_Port GPIOA
#define USART1_TX_Rpi_Pin GPIO_PIN_9
#define USART1_TX_Rpi_GPIO_Port GPIOA
#define USART1_RX_Rpi_Pin GPIO_PIN_10
#define USART1_RX_Rpi_GPIO_Port GPIOA
#define R2_Pin GPIO_PIN_11
#define R2_GPIO_Port GPIOA
#define F2_Pin GPIO_PIN_12
#define F2_GPIO_Port GPIOA
#define UART4_TX_ELRS_Pin GPIO_PIN_10
#define UART4_TX_ELRS_GPIO_Port GPIOC
#define UART4_RX_ELRS_Pin GPIO_PIN_11
#define UART4_RX_ELRS_GPIO_Port GPIOC
#define TRIG1_Pin GPIO_PIN_1
#define TRIG1_GPIO_Port GPIOD
#define ECHO2_Pin GPIO_PIN_2
#define ECHO2_GPIO_Port GPIOD
#define TRIG2_Pin GPIO_PIN_3
#define TRIG2_GPIO_Port GPIOD
#define ECHO3_Pin GPIO_PIN_4
#define ECHO3_GPIO_Port GPIOD
#define TRIG3_Pin GPIO_PIN_5
#define TRIG3_GPIO_Port GPIOD
#define ECHO4_Pin GPIO_PIN_6
#define ECHO4_GPIO_Port GPIOD
#define TRIG4_Pin GPIO_PIN_7
#define TRIG4_GPIO_Port GPIOD
#define ECHO1_Pin GPIO_PIN_3
#define ECHO1_GPIO_Port GPIOB
#define ENCODER_1_B_Pin GPIO_PIN_4
#define ENCODER_1_B_GPIO_Port GPIOB
#define ENCODER_1_A_Pin GPIO_PIN_5
#define ENCODER_1_A_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
