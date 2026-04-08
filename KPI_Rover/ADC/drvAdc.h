#ifndef DRV_ADC_H
#define DRV_ADC_H

#include <stdint.h>

#define SAMPLES_PER_CYCLE 36
#define MAX_HW_ADC_ID     19

typedef void (*drvAdc_Callback)(uint8_t channel, uint32_t filteredRaw, void* ctx);

void drvAdc_init(uint32_t enabledChannels_mask);
void drvAdc_startMeasurement(void);
void drvAdc_stopMeasurement(void);
void drvAdc_registerCallback(uint8_t channel, drvAdc_Callback cb, void* ctx);

#endif // DRV_ADC_H
