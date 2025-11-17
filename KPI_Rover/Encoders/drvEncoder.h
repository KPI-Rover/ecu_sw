#ifndef ENCODERS_DRVENCODER_H_
#define ENCODERS_DRVENCODER_H_

#include "main.h"

void drvEncoder_Init(void);
uint32_t drvEncoder_Read(uint8_t channel);

#endif /* ENCODERS_DRVENCODER_H_ */
