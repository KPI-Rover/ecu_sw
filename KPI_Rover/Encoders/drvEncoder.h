#ifndef ENCODERS_DRVENCODER_H_
#define ENCODERS_DRVENCODER_H_

#include "main.h"

void EncoderDriver_Init(void);
uint32_t EncoderDriver_Read(uint8_t channel);

#endif /* ENCODERS_DRVENCODER_H_ */
