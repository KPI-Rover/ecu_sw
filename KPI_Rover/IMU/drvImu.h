#ifndef IMU_DRVIMU_H_
#define IMU_DRVIMU_H_

#include "main.h"


#define MPU6050_ADDR 			(0x68 << 1)

#define REG_WHO_AM_I  			0x75
#define REG_PWR_MGMT_1 			0x6B
#define REG_GYRO_CONFIG     	0x1B
#define REG_ACCEL_CONFIG    	0x1C
#define REG_DLPF_CONFIG         0x1A
#define REG_ACCEL_XOUT_H 		0x3B

#define ACCEL_SENS_2G       	16384.0f
#define ACCEL_SENS_4G       	8192.0f
#define ACCEL_SENS_8G       	4096.0f
#define ACCEL_SENS_16G       	2048.0f

#define GYRO_SENS_250s       	131.0f
#define GYRO_SENS_500s       	65.5f
#define GYRO_SENS_1000s       	32.8f
#define GYRO_SENS_2000s       	16.4f


typedef enum {
    MPU_ACC_2G  = 0x00,
    MPU_ACC_4G  = 0x01,
    MPU_ACC_8G  = 0x02,
    MPU_ACC_16G = 0x03
} MPU_AccelRange_e;

typedef enum {
    MPU_GYRO_250s  = 0x00,
    MPU_GYRO_500s  = 0x01,
    MPU_GYRO_1000s = 0x02,
    MPU_GYRO_2000s = 0x03
} MPU_GyroRange_e;

typedef struct {
    MPU_AccelRange_e AccelRange;
    MPU_GyroRange_e  GyroRange;
    uint8_t          DlpfConfig;
} MPU_Config_t;

typedef struct {
    int16_t Accel_X, Accel_Y, Accel_Z;
    int16_t Temp; // if we need
    int16_t Gyro_X, Gyro_Y, Gyro_Z;
} MPU_RawData_t;


HAL_StatusTypeDef drvImu_Init(I2C_HandleTypeDef *hi2c, MPU_Config_t *config);
HAL_StatusTypeDef drvImu_GetRawData(I2C_HandleTypeDef *hi2c, MPU_RawData_t *pRawData);

#endif /* IMU_DRVIMU_H_ */
