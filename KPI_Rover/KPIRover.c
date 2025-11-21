#include "KPIRover.h"
#include "cmsis_os.h"

#include "Database/ulDatabase.h"


static struct ulDatabase_ParamMetadata ulDatabase_params[] = {
};

void KPIRover_Init(void) {
	ulDatabase_init(ulDatabase_params, sizeof(ulDatabase_params) / sizeof(struct ulDatabase_ParamMetadata));
}
