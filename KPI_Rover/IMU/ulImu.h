#ifndef IMU_ULIMU_H_
#define IMU_ULIMU_H_

#include "drvImu.h"


#define GRAVITY_MS2		9.81f


HAL_StatusTypeDef ulImu_Init(I2C_HandleTypeDef *hi2c);

#endif /* IMU_ULIMU_H_ */
