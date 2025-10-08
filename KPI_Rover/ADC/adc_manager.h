#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include "adc_driver.h"
#include "FreeRTOS.h"
#include "queue.h"


typedef struct {
    uint8_t channel;
    float phys_high;
    uint8_t calibration_required;
} adc_channel_config_t;

void ADC_Manager_Init(void);
void ADC_Manager_Task(void *argument);

#endif // ADC_MANAGER_H
