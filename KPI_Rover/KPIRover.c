#include "KPIRover.h"
#include "cmsis_os.h"

#include "Database/ulDatabase.h"
#include "Database/ulStorage.h"
#include "Encoders/ulEncoder.h"

#include "ulog.h"
#include "ul_ulog.h"

static struct ulDatabase_ParamMetadata ulDatabase_params[] = {
		{0, UINT16, true, 5}, // ENCODER_CONTROL_PERIOD_MS,
		{0, FLOAT, true, 820.0f}, // ENCODER_TICKS_PER_REVOLUTION,
		{0, INT32, false, 0}, // MOTOR_FL_RPM,
		{0, INT32, false, 0}, // MOTOR_FR_RPM,
		{0, INT32, false, 0}, // MOTOR_RL_RPM,
		{0, INT32, false, 0}, // MOTOR_RR_RPM,
};

void KPIRover_Init(void) {
	ULOG_INIT();
	ULOG_SUBSCRIBE(ul_ulog_send, ULOG_DEBUG_LEVEL);
	ulDatabase_init(ulDatabase_params, sizeof(ulDatabase_params) / sizeof(struct ulDatabase_ParamMetadata));
	ulStorage_init();
	ulEncoder_Init();
}
