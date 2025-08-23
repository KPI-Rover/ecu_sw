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

void ADC_PerformTwoPointCalibration(uint8_t channel) {
    adc_calibration_t calib = {0};

    osDelay(500);

    ULOG_INFO("Calibration: 0 В on channel %u", channel);
    uint32_t sum = 0;
    uint32_t count = 0;
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < 5000) {
        sum += ADC_Driver_GetLastValue(channel);
        count++;
        osDelay(100);
    }
    calib.raw_low = (count > 0) ? (uint16_t)(sum / count) : 0;
    calib.phys_low = 0.0f;
    ULOG_INFO("raw_low = %u", calib.raw_low);

    osDelay(100);

    ULOG_INFO("Wait 10s, 3.3 В on channel %u", channel);
    osDelay(10000);

    sum = 0;
    count = 0;
    start = HAL_GetTick();
    while (HAL_GetTick() - start < 5000) {
        sum += ADC_Driver_GetLastValue(channel);
        count++;
        osDelay(100);
    }
    calib.raw_high = (count > 0) ? (uint16_t)(sum / count) : 0;
    calib.phys_high = 3.3f;
    ULOG_INFO("raw_high = %u", calib.raw_high);

    ADC_Driver_SetCalibration(channel, calib);

    ULOG_INFO("Calibration complete: raw_low=%u, raw_high=%u",
              calib.raw_low, calib.raw_high);
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
