#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include "cmsis_os.h"

typedef struct {
    uint16_t raw_low;
    uint16_t raw_high;
    float phys_low;
    float phys_high;
} adc_calibration_t;

typedef void (*adc_callback_t)(uint8_t channel, uint16_t value);

void ADC_Driver_Init(const uint8_t* channels, size_t count);
void ADC_Driver_Start(void);
void ADC_PerformTwoPointCalibration(uint8_t channel);
void ADC_Driver_SetCalibration(uint8_t channel, adc_calibration_t calib);
float ADC_Driver_GetCalibratedValue(uint8_t channel);
void ADC_Driver_NotifyTaskOnConversion(TaskHandle_t taskHandle);
uint16_t ADC_Driver_GetLastValue(uint8_t channel);
void ADC_Driver_RegisterCallback(adc_callback_t cb);

#endif
