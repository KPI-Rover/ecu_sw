#ifndef ULOG_TASK_H_
#define UL_ULOG_H_

#ifdef __cplusplus
extern "C" {
    #endif

#include "ulog.h"

void ul_ulog_init();
void ul_ulog_send(ulog_level_t level, const char *filename, char *msg);

#ifdef __cplusplus
}
#endif

#endif /* UL_ULOG_H_ */
