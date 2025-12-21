#ifndef COMMUNICATION_PROTOCOLHANDLER_H_
#define COMMUNICATION_PROTOCOLHANDLER_H_

#include "stdint.h"

void ProtocolHandler_run();
void ProtocolHandler_processRequest(uint8_t *request);

#endif /* COMMUNICATION_PROTOCOLHANDLER_H_ */
