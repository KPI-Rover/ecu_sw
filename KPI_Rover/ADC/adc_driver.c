#include "adc_driver.h"
#include "stm32f4xx_hal.h"
#include "ulog.h"

#define MAX_ADC_CHANNELS 16
#define EMA_ALPHA 0.2f

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

void ADC_Driver_Calibrate(void) {
    for (size_t i = 0; i < channel_count; i++) {
        adc_filtered_ema[i] = 0.0f;
        adc_filtered_values[i] = 0;
        adc_raw_values[i] = 0;
    }
    ULOG_INFO("ADC calibration: filters and raw values reset");
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
            float raw = (float)adc_raw_values[i];
            adc_filtered_ema[i] = EMA_ALPHA * raw + (1.0f - EMA_ALPHA) * adc_filtered_ema[i];
            adc_filtered_values[i] = (uint16_t)adc_filtered_ema[i];
        }


        if (notify_task) {
            vTaskNotifyGiveFromISR(notify_task, &xHigherPriorityTaskWoken);
        }

        if (registered_callback) {
            for (size_t i = 0; i < channel_count; i++) {
                registered_callback(adc_channels[i], adc_filtered_values[i]);
            }
            ULOG_DEBUG("ADC callback invoked");
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
