#include "ulImu.h"
#include "cmsis_os.h"
#include "ulog.h"
#include "Database/ulDatabase.h"


#define IMU_POLL_PERIOD_MS 20


static osTimerId_t imuTimerHandle;

static float accel_bias[3] = {0.0f, 0.0f, 0.0f};
static float gyro_bias[3]  = {0.0f, 0.0f, 0.0f};


static MPU_Config_t imuCfg = {
    .AccelRange = MPU_ACC_2G,
    .GyroRange  = MPU_GYRO_250s,
    .DlpfConfig = 0x03  // 44 Hz filtering, 4.8 delay
};

static const float AccelScales[] = {
    ACCEL_SENS_2G,
    ACCEL_SENS_4G,
    ACCEL_SENS_8G,
    ACCEL_SENS_16G
};

static const float GyroScales[] = {
    GYRO_SENS_250s,
    GYRO_SENS_500s,
    GYRO_SENS_1000s,
    GYRO_SENS_2000s
};


void ulImu_Update(I2C_HandleTypeDef *hi2c);


static void ulImu_CalibrateGyro(I2C_HandleTypeDef *hi2c) {
    // TODO calibrate
    ulDatabase_setUint8(IMU_IS_CALIBRATED, 1);
}

void ulImu_TimerCallback(void *argument) {
	I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)argument;
	ulImu_Update(hi2c);
}

HAL_StatusTypeDef ulImu_Init(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef status;

    status = drvImu_Init(hi2c, &imuCfg);
    if (status != HAL_OK) {
    	return status;
    }

    ulImu_CalibrateGyro(hi2c);

    const osTimerAttr_t imuTimer_attributes = {
    	.name = "IMUTimer"
    };

    imuTimerHandle = osTimerNew(ulImu_TimerCallback, osTimerPeriodic, (void*)hi2c, &imuTimer_attributes);
    if (imuTimerHandle == NULL) {
        ULOG_ERROR("Failed to create IMU timer");
        return HAL_ERROR;
    }
    if (osTimerStart(imuTimerHandle, IMU_POLL_PERIOD_MS) != osOK) {
        ULOG_ERROR("Failed to start IMU timer");
        return HAL_ERROR;
    }

    return HAL_OK;
}

void ulImu_Update(I2C_HandleTypeDef *hi2c) {
    MPU_RawData_t raw;
    uint8_t is_calibrate;

    if (ulDatabase_getUint8(IMU_IS_CALIBRATED, &is_calibrate) && is_calibrate == 0) {
        ulImu_CalibrateGyro(hi2c);
        return;
    }

    if (drvImu_GetRawData(hi2c, &raw) != HAL_OK) {
    	return;
    }

    float ax_ms2 = (raw.Accel_X - accel_bias[0]) / AccelScales[imuCfg.AccelRange] * GRAVITY_MS2;
    float ay_ms2 = (raw.Accel_Y - accel_bias[1]) / AccelScales[imuCfg.AccelRange] * GRAVITY_MS2;
    float az_ms2 = (raw.Accel_Z - accel_bias[2]) / AccelScales[imuCfg.AccelRange] * GRAVITY_MS2;

    float gx_angle = (raw.Gyro_X - gyro_bias[0]) / GyroScales[imuCfg.GyroRange];
    float gy_angle = (raw.Gyro_Y - gyro_bias[1]) / GyroScales[imuCfg.GyroRange];
    float gz_angle = (raw.Gyro_Z - gyro_bias[2]) / GyroScales[imuCfg.GyroRange];


    ulDatabase_setFloat(IMU_ACCEL_X, ax_ms2);
    ulDatabase_setFloat(IMU_ACCEL_Y, ay_ms2);
    ulDatabase_setFloat(IMU_ACCEL_Z, az_ms2);

    ulDatabase_setFloat(IMU_GYRO_X, gx_angle);
    ulDatabase_setFloat(IMU_GYRO_Y, gy_angle);
    ulDatabase_setFloat(IMU_GYRO_Z, gz_angle);
}
