#include "KPIRover.h"
#include "cmsis_os.h"

#include "Database/ulDatabase.h"
#include "Database/ulStorage.h"
#include "Encoders/ulEncoder.h"
#include "Communication/ProtocolHandler.h"
#include "IMU/ulImu.h"

#include "ulog.h"
#include "ul_ulog.h"

static struct ulDatabase_ParamMetadata ulDatabase_params[] = {
		{0, UINT16, true, 5}, // ENCODER_CONTROL_PERIOD_MS,
		{0, FLOAT, true, 820.0f}, // ENCODER_TICKS_PER_REVOLUTION,
		{0, INT32, false, 0}, // ENCODER_MOTOR_FL_RPM,
		{0, INT32, false, 0}, // ENCODER_MOTOR_FR_RPM,
		{0, INT32, false, 0}, // ENCODER_MOTOR_RL_RPM,
		{0, INT32, false, 0}, // ENCODER_MOTOR_RR_RPM,
		{0, INT32, false, 0}, // TARGET_MOTOR_FL_RPM,
		{0, INT32, false, 0}, // TARGET_MOTOR_FR_RPM,
		{0, INT32, false, 0}, // TARGET_MOTOR_RL_RPM,
		{0, INT32, false, 0}, // TARGET_MOTOR_RR_RPM,
		{0, FLOAT, false, 0.0f}, // IMU_ACCEL_X
		{0, FLOAT, false, 0.0f}, // IMU_ACCEL_Y
		{0, FLOAT, false, 0.0f}, // IMU_ACCEL_Z
		{0, FLOAT, false, 0.0f}, // IMU_GYRO_X
		{0, FLOAT, false, 0.0f}, // IMU_GYRO_Y
		{0, FLOAT, false, 0.0f}, // IMU_GYRO_Z
		{0, UINT8, false, 0}, // IMU_CALIB_CMD
		{0, UINT8, false, 0}, // IMU_CALIB_STATUS
		{0, UINT8, false, 0}, // IMU_IS_CALIBRATED
};

extern I2C_HandleTypeDef hi2c3;

void KPIRover_Init(void) {
	ULOG_INIT();
	ULOG_SUBSCRIBE(ul_ulog_send, ULOG_DEBUG_LEVEL);
	ulDatabase_init(ulDatabase_params, sizeof(ulDatabase_params) / sizeof(struct ulDatabase_ParamMetadata));
	ulStorage_init();
	ulEncoder_Init();
	ulImu_Init(&hi2c3);
	ProtocolHandler_init();
}
