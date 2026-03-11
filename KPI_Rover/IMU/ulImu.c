#include "ulImu.h"
#include "cmsis_os.h"
#include "ulog.h"
#include "Database/ulDatabase.h"
#include <float.h>


#define IMU_POLL_PERIOD_MS 20

#define CALIB_SAMPLES 100


static osTimerId_t imuTimerHandle;


static float accel_bias[3] = {0.0f, 0.0f, 0.0f};
static float gyro_bias[3]  = {0.0f, 0.0f, 0.0f};

static int32_t calib_sum[3] = {0, 0, 0};
static uint16_t calib_count = 0;

static float acc_max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
static float acc_min[3] = {FLT_MAX, FLT_MAX, FLT_MAX};

static float accel_scale_corr[3] = {1.0f, 1.0f, 1.0f};


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


static void ulImu_Update(I2C_HandleTypeDef *hi2c);


static void ulImu_Calibration(I2C_HandleTypeDef *hi2c, uint8_t cmd) {
    MPU_RawData_t raw;
    uint8_t current_status = STATUS_IDLE;
    ulDatabase_getUint8(IMU_CALIB_STATUS, &current_status);

    if (current_status == STATUS_IDLE) {
    	calib_sum[0] = 0;
    	calib_sum[1] = 0;
    	calib_sum[2] = 0;
    	calib_count = 0;
        ulDatabase_setUint8(IMU_CALIB_STATUS, STATUS_IN_PROGRESS);
        return;
    }

    if (current_status == STATUS_IN_PROGRESS) {

    	if (cmd == CMD_CALIB_APPLY) {

    		for (int i = 0; i < 3; i++) {
    			if (acc_max[i] != -FLT_MAX && acc_min[i] != FLT_MAX) {
    				accel_bias[i] = (acc_max[i] + acc_min[i]) / 2.0f;

    				float measured_1g = (acc_max[i] - acc_min[i]) / 2.0f;
    				float expected_1g = AccelScales[imuCfg.AccelRange];

    				if (measured_1g > (expected_1g * 0.5f)) {
    					accel_scale_corr[i] = measured_1g / AccelScales[imuCfg.AccelRange];
            		}
    			}

    			acc_max[i] = -FLT_MAX;
    			acc_min[i] = FLT_MAX;
            }

            ulDatabase_setUint8(IMU_CALIB_STATUS, STATUS_DONE);
            ulDatabase_setUint8(IMU_IS_CALIBRATED, 1);
            return;
        }

        if (drvImu_GetRawData(hi2c, &raw) != HAL_OK) {
        	ulDatabase_setUint8(IMU_CALIB_STATUS, STATUS_ERROR);
        	ULOG_ERROR("IMU: Read I2C data failed");
        	return;
        }

        if (cmd == CMD_CALIB_GYRO) {
            calib_sum[0] += raw.Gyro_X;
            calib_sum[1] += raw.Gyro_Y;
            calib_sum[2] += raw.Gyro_Z;
        }
        else if (cmd >= CMD_CALIB_ACC_Z_POS && cmd <= CMD_CALIB_ACC_Y_NEG) {
            calib_sum[0] += raw.Accel_X;
            calib_sum[1] += raw.Accel_Y;
            calib_sum[2] += raw.Accel_Z;
        }

        calib_count++;

        if (calib_count >= CALIB_SAMPLES) {
            float avg_x = (float)calib_sum[0] / CALIB_SAMPLES;
            float avg_y = (float)calib_sum[1] / CALIB_SAMPLES;
            float avg_z = (float)calib_sum[2] / CALIB_SAMPLES;

            if (cmd == CMD_CALIB_GYRO) {
                gyro_bias[0] = avg_x;
                gyro_bias[1] = avg_y;
                gyro_bias[2] = avg_z;
            }
            else {

                float avgs[3] = {avg_x, avg_y, avg_z};

                for (int i = 0; i < 3; i++) {
                    if (avgs[i] > acc_max[i]) {
                    	acc_max[i] = avgs[i];
                    }
                    if (avgs[i] < acc_min[i]) {
                    	acc_min[i] = avgs[i];
                    }
                }
            }

            ulDatabase_setUint8(IMU_CALIB_STATUS, STATUS_DONE);
            ULOG_INFO("IMU: Calibration is done");
        }
    }
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

static void ulImu_Update(I2C_HandleTypeDef *hi2c) {
    uint8_t cmd = CMD_IDLE;

    if (ulDatabase_getUint8(IMU_CALIB_CMD, &cmd) && cmd != CMD_IDLE) {
    	ULOG_INFO("IMU: Started calibration");
    	ulImu_Calibration(hi2c, cmd);
    	return;
    }
    else {
        uint8_t status = STATUS_IDLE;
        ulDatabase_getUint8(IMU_CALIB_STATUS, &status);

        if (status != STATUS_IDLE) {
            ulDatabase_setUint8(IMU_CALIB_STATUS, STATUS_IDLE);
        }
    }

    MPU_RawData_t raw;
    if (drvImu_GetRawData(hi2c, &raw) != HAL_OK) {
    	ULOG_ERROR("IMU: Read I2C data failed");
    	return;
    }

    float ax_ms2 = (raw.Accel_X - accel_bias[0]) / (AccelScales[imuCfg.AccelRange] * accel_scale_corr[0]) * GRAVITY_MS2;
    float ay_ms2 = (raw.Accel_Y - accel_bias[1]) / (AccelScales[imuCfg.AccelRange] * accel_scale_corr[1]) * GRAVITY_MS2;
    float az_ms2 = (raw.Accel_Z - accel_bias[2]) / (AccelScales[imuCfg.AccelRange] * accel_scale_corr[2]) * GRAVITY_MS2;

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
