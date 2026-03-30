#ifndef IMU_ULIMU_H_
#define IMU_ULIMU_H_

#include "drvImu.h"


#define GRAVITY_MS2		9.81f


typedef enum {
    CMD_IDLE = 0,
    CMD_CALIB_GYRO = 1,
    CMD_CALIB_ACC_Z_POS = 2,
    CMD_CALIB_ACC_Z_NEG = 3,
    CMD_CALIB_ACC_X_POS = 4,
    CMD_CALIB_ACC_X_NEG = 5,
    CMD_CALIB_ACC_Y_POS = 6,
    CMD_CALIB_ACC_Y_NEG = 7,
    CMD_CALIB_APPLY = 8
} ImuCalibCmd_t;

typedef enum {
    STATUS_IDLE = 0,
    STATUS_IN_PROGRESS = 1,
    STATUS_DONE = 2,
    STATUS_ERROR = 3
} ImuCalibStatus_t;


HAL_StatusTypeDef ulImu_Init(I2C_HandleTypeDef *hi2c);

#endif /* IMU_ULIMU_H_ */
