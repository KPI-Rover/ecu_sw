#include <Motors/ulMotorsController.h>
#include "KPIRover.h"
#include "cmsis_os.h"
#include "Database/ulDatabase.h"


static struct ulDatabase_ParamMetadata ulDatabase_params[] = {
};

void KPIRover_Init(void) {
	ulDatabase_init(ulDatabase_params, sizeof(ulDatabase_params) / sizeof(struct ulDatabase_ParamMetadata));

	static const osThreadAttr_t MotorsCtrlTask_attributes = {
	      .name = "MotorsCtrlTask",
	      .priority = (osPriority_t) osPriorityNormal,
	      .stack_size = 1024 * 4
	  };
	  (void) osThreadNew(ulMotorsController_Task, NULL, &MotorsCtrlTask_attributes);
}
