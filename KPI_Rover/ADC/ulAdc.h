#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#include <stdint.h>

float ulAdc_applyCalibration(uint8_t channel, uint32_t rawValue);

void ulAdc_init(void);
void ulAdc_task(void *argument);

#endif // ADC_MANAGER_H
