#include "KPIRover.h"
#include "cmsis_os.h"

#include "Database/ulDatabase.h"


static struct ulDatabase_ParamMetadata ulDatabase_params[] = {
		{0, INT32, false, 0}, // MOTOR_FL_RPM,
		{0, INT32, false, 0}, // MOTOR_FR_RPM,
		{0, INT32, false, 0}, // MOTOR_RL_RPM,
		{0, INT32, false, 0}, // MOTOR_RR_RPM,
		{0, UINT16, true, 5}, // ENCODER_CONTROL_PERIOD_MS,
		{0, FLOAT, true, 820.0f}, // ENCODER_TICKS_PER_REVOLUTION,
		{0, FLOAT, false, 0.0f}, // IMU_ACCEL_X
		{0, FLOAT, false, 0.0f}, // IMU_ACCEL_Y
		{0, FLOAT, false, 0.0f}, // IMU_ACCEL_Z
		{0, FLOAT, false, 0.0f}, // IMU_GYRO_X
		{0, FLOAT, false, 0.0f}, // IMU_GYRO_Y
		{0, FLOAT, false, 0.0f}, // IMU_GYRO_Z
		{0, UINT8, false, 0}, // IMU_IS_CALIBRATED
};

void KPIRover_Init(void) {
	ulDatabase_init(ulDatabase_params, sizeof(ulDatabase_params) / sizeof(struct ulDatabase_ParamMetadata));
	ulEncoder_Init();
}
