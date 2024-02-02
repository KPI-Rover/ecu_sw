#include "HeartbeatTask.h"
#include "main.h"

#include "cmsis_os.h"


static bool isError = true;

void HeartbeatTask(void const *argument) {
    for(;;) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        if(isError) {
            osDelay(1000); // Швидке миготіння
        } else {
            osDelay(3000); // Повільне миготіння
        }
    }
}

void setHeartbeatError(bool errorStatus) {
    isError = errorStatus;
}
