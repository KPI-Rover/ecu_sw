#include "hal_motor.h"

#define MOTOR1_PWM_CHANNEL TIM1_CH1_PWM1
#define MOTOR2_PWM_CHANNEL TIM1_CH2_PWM2
#define MOTOR3_PWM_CHANNEL TIM1_CH3_PWM3
#define MOTOR4_PWM_CHANNEL TIM1_CH4_PWM4

#define MOTOR1_F_PIN F1
#define MOTOR1_R_PIN R1
#define MOTOR2_F_PIN F2
#define MOTOR2_R_PIN R2
#define MOTOR3_F_PIN F3
#define MOTOR3_R_PIN R3
#define MOTOR4_F_PIN F4
#define MOTOR4_R_PIN R4

void SetMotorPWM(TIM_HandleTypeDef* htim, uint32_t channel, uint16_t duty) {
    __HAL_TIM_SET_COMPARE(htim, channel, duty);
}

void SetMotorDirection(int motor, int direction) {
    GPIO_TypeDef* portF;
    uint16_t pinF;
    GPIO_TypeDef* portR;
    uint16_t pinR;

    switch (motor) {
        case 1:
            portF = GPIOB;
            pinF = MOTOR1_F_PIN;
            portR = GPIOB;
            pinR = MOTOR1_R_PIN;
            break;
        case 2:
            portF = GPIOB;
            pinF = MOTOR2_F_PIN;
            portR = GPIOB;
            pinR = MOTOR2_R_PIN;
            break;
        case 3:
            portF = GPIOB;
            pinF = MOTOR3_F_PIN;
            portR = GPIOB;
            pinR = MOTOR3_R_PIN;
            break;
        case 4:
            portF = GPIOB;
            pinF = MOTOR4_F_PIN;
            portR = GPIOB;
            pinR = MOTOR4_R_PIN;
            break;
        default:
            return;
    }

    if (direction == 1) { // Forward
        HAL_GPIO_WritePin(portF, pinF, GPIO_PIN_SET);
        HAL_GPIO_WritePin(portR, pinR, GPIO_PIN_RESET);
    } else if (direction == -1) { // Reverse
        HAL_GPIO_WritePin(portF, pinF, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portR, pinR, GPIO_PIN_SET);
    } else { // Stop
        HAL_GPIO_WritePin(portF, pinF, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portR, pinR, GPIO_PIN_RESET);
    }
}
