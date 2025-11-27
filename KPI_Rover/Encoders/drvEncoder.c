#include "drvEncoder.h"


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;


void drvEncoder_Init(void) {
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

uint32_t drvEncoder_Read(uint8_t channel) {
    switch (channel) {
        case 0: return TIM1->CNT;
        case 1: return TIM2->CNT;
        case 2: return TIM3->CNT;
        case 3: return TIM4->CNT;
        default: return 0;
    }
}
