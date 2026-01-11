#include "drvImu.h"


#define I2C_TIMEOUT 100


HAL_StatusTypeDef drvImu_Init(I2C_HandleTypeDef *hi2c, MPU_Config_t *config) {
	uint8_t check;
	uint8_t data;
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, REG_WHO_AM_I, 1, &check, 1, I2C_TIMEOUT);
	if (status != HAL_OK || check != 0x68) {
		return HAL_ERROR;
	}

	data = 0x00;
	status = HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, REG_PWR_MGMT_1, 1, &data, 1, I2C_TIMEOUT);
    if (status != HAL_OK) {
    	return status;
    }

    data = config->DlpfConfig; // 44 Hz filtering, 4.8 delay
    status = HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, REG_DLPF_CONFIG, 1, &data, 1, I2C_TIMEOUT);
    if (status != HAL_OK) {
    	return status;
    }

    data = (config->GyroRange << 3);
    status = HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, REG_GYRO_CONFIG, 1, &data, 1, I2C_TIMEOUT);
    if (status != HAL_OK) {
    	return status;
    }

    data = (config->AccelRange << 3);
    status = HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, REG_ACCEL_CONFIG, 1, &data, 1, I2C_TIMEOUT);

    return status;
}

HAL_StatusTypeDef drvImu_GetRawData(I2C_HandleTypeDef *hi2c, MPU_RawData_t *pRawData) {
    uint8_t buffer[14];
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, REG_ACCEL_XOUT_H, 1, buffer, 14, I2C_TIMEOUT);

    if (status == HAL_OK) {
        pRawData->Accel_X = (int16_t)(buffer[0] << 8 | buffer[1]);
        pRawData->Accel_Y = (int16_t)(buffer[2] << 8 | buffer[3]);
        pRawData->Accel_Z = (int16_t)(buffer[4] << 8 | buffer[5]);

        // if we need
        pRawData->Temp    = (int16_t)(buffer[6] << 8 | buffer[7]);

        pRawData->Gyro_X  = (int16_t)(buffer[8] << 8 | buffer[9]);
        pRawData->Gyro_Y  = (int16_t)(buffer[10] << 8 | buffer[11]);
        pRawData->Gyro_Z  = (int16_t)(buffer[12] << 8 | buffer[13]);
    }

    return status;
}

