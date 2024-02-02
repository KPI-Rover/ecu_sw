#ifndef HEARTBEATTASK_H
#define HEARTBEATTASK_H

#include "cmsis_os.h"

#ifndef bool
typedef enum { false, true } bool;
#endif

void HeartbeatTask(void const * argument);
void setHeartbeatError(bool errorStatus);

#endif
