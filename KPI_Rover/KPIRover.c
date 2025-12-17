#include <Motors/ulMotorsController.h>
#include <Encoders/ulEncoder.h>
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
		{0, FLOAT, true, 0.046f}, // MOTOR_FL_KP
		{0, FLOAT, true, 0.013f}, // MOTOR_FL_KI
		{0, FLOAT, true, 0.0001f}, // MOTOR_FL_KD
		{0, FLOAT, true, 0.046f}, // MOTOR_FR_KP
		{0, FLOAT, true, 0.013f}, // MOTOR_FR_KI
		{0, FLOAT, true, 0.0001f}, // MOTOR_FR_KD
		{0, FLOAT, true, 0.046f}, // MOTOR_RL_KP
		{0, FLOAT, true, 0.013f}, // MOTOR_RL_KI
		{0, FLOAT, true, 0.0001f}, // MOTOR_RL_KD
		{0, FLOAT, true, 0.046f}, // MOTOR_RR_KP
		{0, FLOAT, true, 0.013f}, // MOTOR_RR_KI
		{0, FLOAT, true, 0.0001f}, // MOTOR_RR_KD
		{0, INT32, false, 0}, // MOTOR_FL_SETPOINT,
		{0, INT32, false, 0}, // MOTOR_FR_SETPOINT,
		{0, INT32, false, 0}, // MOTOR_RL_SETPOINT,
		{0, INT32, false, 0}, // MOTOR_RR_SETPOINT,
		{0, INT32, false, 0}, // MOTOR_FL_RPM,
		{0, INT32, false, 0}, // MOTOR_FR_RPM,
		{0, INT32, false, 0}, // MOTOR_RL_RPM,
		{0, INT32, false, 0}, // MOTOR_RR_RPM,
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

	static const osThreadAttr_t MotorsCtrlTask_attributes = {
		.name = "MotorsCtrlTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 1024 * 4
	};
	(void) osThreadNew(ulMotorsController_Task, NULL, &MotorsCtrlTask_attributes);

}
