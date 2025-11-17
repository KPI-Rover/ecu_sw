#include "drvEncoder.h"


extern TIM_HandleTypeDef htim2;
/*other encoders
extern TIM_HandleTypeDef htim?;
extern TIM_HandleTypeDef htim?;
extern TIM_HandleTypeDef htim?;
*/

void drvEncoder_Init(void) {
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    /* other encoders
    HAL_TIM_Encoder_Start(&htim?, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim?, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim?, TIM_CHANNEL_ALL);
    */
}

uint32_t drvEncoder_Read(uint8_t channel) {
    switch (channel) {
        case 0: return TIM2->CNT;
        /* other encoders
        case 1: return TIM?->CNT;
        case 2: return TIM?->CNT;
        case 3: return TIM?->CNT;
        */
        default: return 0;
    }
}
