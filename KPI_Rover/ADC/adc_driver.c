#include "adc_driver.h"
#include "stm32f4xx_hal.h"
#include "ulog.h"

#define MAX_ADC_CHANNELS 16
#define EMA_ALPHA 0.05f

static uint8_t adc_channels[MAX_ADC_CHANNELS];
static uint16_t adc_raw_values[MAX_ADC_CHANNELS];
static uint16_t adc_filtered_values[MAX_ADC_CHANNELS];
static size_t channel_count = 0;

static TaskHandle_t notify_task = NULL;
static adc_callback_t registered_callback = NULL;

static adc_calibration_t adc_calibration_data[MAX_ADC_CHANNELS];

static float adc_filtered_ema[MAX_ADC_CHANNELS] = {0.0f};

extern ADC_HandleTypeDef hadc1;

void ADC_Driver_Init(const uint8_t* channels, size_t count) {
	channel_count = count > MAX_ADC_CHANNELS ? MAX_ADC_CHANNELS : count;
    for (size_t i = 0; i < channel_count; i++) {
    	adc_channels[i] = channels[i];
        ULOG_INFO("ADC channel %d initialized", channels[i]);
    }
    ULOG_INFO("ADC driver initialized with %d channels", channel_count);
}

void ADC_Driver_Start(void) {
	if (channel_count == 0) {
		ULOG_ERROR("ADC start failed: no channels configured");
		return;
	}
	if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw_values, channel_count) != HAL_OK){
		ULOG_ERROR("Failed to start ADC DMA");
	}else {
		ULOG_INFO("ADC DMA started with %d channels", channel_count);
	}
}

uint16_t ADC_PerformTwoPointCalibration(uint8_t channel) {
	uint16_t samples[36];
	uint32_t start = HAL_GetTick();
	size_t count = 0;

	while ((HAL_GetTick() - start < 100) && (count < 36)) {
		samples[count++] = ADC_Driver_GetLastValue(channel);
		osDelay(3);
	}

	uint16_t min1=0xFFFF, min2=0xFFFF, max1=0, max2=0;
	for (size_t i=0; i<count; i++) {
		uint16_t v = samples[i];
		if (v < min1) {
			min2=min1;
			min1=v;
		}
		else if (v < min2) {
			min2=v;
		}

		if (v > max1) {
			max2=max1;
			max1=v;
		}
		else if (v > max2) {
			max2=v;
		}
	}

	uint16_t avg_min = (min1+min2)/2;
	uint16_t avg_max = (max1+max2)/2;

	uint32_t sum = 0;
	for (size_t i=0; i<count; i++) sum += samples[i];
	uint16_t avg_all = sum / count;

	return (avg_min + avg_max + avg_all)/3;

}

void ADC_Driver_StartCalibrationLow(uint8_t channel) {
    uint16_t raw = ADC_PerformTwoPointCalibration(channel);
    adc_calibration_t calib = adc_calibration_data[channel];
    calib.raw_low = raw;
    calib.phys_low = 0.0f;

    ADC_Driver_SetCalibration(channel, calib);
    ULOG_INFO("Calibration LOW complete: raw=%u", raw);
}

void ADC_Driver_StartCalibrationHigh(uint8_t channel) {
    uint16_t raw = ADC_PerformTwoPointCalibration(channel);
    adc_calibration_t calib = adc_calibration_data[channel];
    calib.raw_high = raw;
    calib.phys_high = 3.3f;

    ADC_Driver_SetCalibration(channel, calib);
    ULOG_INFO("Calibration HIGH complete: raw=%u", raw);
}



void ADC_Driver_SetCalibration(uint8_t channel, adc_calibration_t calib) {
    for (size_t i = 0; i < channel_count; ++i) {
        if (adc_channels[i] == channel) {
            adc_calibration_data[i] = calib;
            ULOG_INFO("Calibration set for channel %u", channel);
            return;
        }
    }
    ULOG_WARNING("Attempted to calibrate unknown channel %u", channel);
}

float ADC_Driver_GetCalibratedValue(uint8_t channel) {
    for (size_t i = 0; i < channel_count; ++i) {
        if (adc_channels[i] == channel) {
            adc_calibration_t calib = adc_calibration_data[i];
            uint16_t raw = adc_filtered_values[i];

            if (calib.raw_high == calib.raw_low) return 0.0f;

            float phys = calib.phys_low + ((float)(raw - calib.raw_low)) *
                         (calib.phys_high - calib.phys_low) /
                         (float)(calib.raw_high - calib.raw_low);
            return phys;
        }
    }

    ULOG_WARNING("Calibrated value requested for unknown channel %u", channel);
    return 0.0f;
}

void ADC_Driver_NotifyTaskOnConversion(TaskHandle_t taskHandle) {
    notify_task = taskHandle;
}

void ADC_Driver_RegisterCallback(adc_callback_t cb) {
    registered_callback = cb;
    ULOG_DEBUG("ADC callback registered");
}

uint16_t ADC_Driver_GetLastValue(uint8_t channel) {
    for (size_t i = 0; i < channel_count; ++i) {
        if (adc_channels[i] == channel)
        	return adc_filtered_values[i];
    }
    ULOG_WARNING("ADC: Requested value for unregistered channel %u", channel);
    return 0;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == hadc1.Instance) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        for (size_t i = 0; i < channel_count; i++) {
            uint16_t raw = adc_raw_values[i];

            if (raw == 4095) {
                adc_filtered_values[i] = raw;
                adc_filtered_ema[i] = (float)raw;
            } else {
                adc_filtered_ema[i] = EMA_ALPHA * (float)raw +
                                      (1.0f - EMA_ALPHA) * adc_filtered_ema[i];
                adc_filtered_values[i] = (uint16_t)adc_filtered_ema[i];
            }
        }

        if (notify_task) {
            vTaskNotifyGiveFromISR(notify_task, &xHigherPriorityTaskWoken);
        }

        if (registered_callback) {
            for (size_t i = 0; i < channel_count; i++) {
                registered_callback(adc_channels[i], adc_filtered_values[i]);
            }
        }

        HAL_ADC_Stop_DMA(&hadc1);
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw_values, channel_count);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
