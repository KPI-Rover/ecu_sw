#include "drvAdc.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include <string.h>

extern ADC_HandleTypeDef hadc1;

#define DRV_ADC_FLAG_DMA_COMPLETE 0x01
#define DRV_ADC_FLAG_TIMER_TRIG   0x02
#define MEASUREMENT_PERIOD_MS     100

typedef enum { ADC_STATE_IDLE, ADC_STATE_BUSY } drv_adc_state_t;

static drv_adc_state_t current_state = ADC_STATE_IDLE;
static uint8_t active_hw_channels[MAX_HW_ADC_ID];
static size_t active_channel_count = 0;

static uint16_t dma_buffer[MAX_HW_ADC_ID * SAMPLES_PER_CYCLE];

static osThreadId_t drv_adc_thread_id = NULL;
static osTimerId_t drv_adc_timer_id = NULL;

typedef struct {
    drvAdc_Callback cb;
    void* ctx;
} adc_callback_entry_t;
static adc_callback_entry_t callback_table[MAX_HW_ADC_ID];


static uint16_t Filter_Algorithm_36Samples(uint16_t* data, size_t step) {
    uint32_t sum = 0;
    uint16_t min1 = 0xFFFF, min2 = 0xFFFF;
    uint16_t max1 = 0, max2 = 0;

    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
        uint16_t val = data[i * step];
        sum += val;

        if (val < min1) {
        	min2 = min1; min1 = val;
        } else if (val < min2) {
        	min2 = val;
        }

        if (val > max1) {
        	max2 = max1; max1 = val;
        } else if (val > max2) {
        	max2 = val;
        }
    }

    sum -= (min1 + min2 + max1 + max2);
    return (uint16_t)(sum >> 5); // / 32
}

static void drvAdc_WorkerTask(void *argument) {
    for (;;) {
        uint32_t flags = osThreadFlagsWait(DRV_ADC_FLAG_TIMER_TRIG | DRV_ADC_FLAG_DMA_COMPLETE, osFlagsWaitAny, osWaitForever);

        switch (current_state) {

            case ADC_STATE_IDLE:
                if (flags & DRV_ADC_FLAG_TIMER_TRIG) {
                    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&hadc1,
                    (uint32_t*)dma_buffer, active_channel_count * SAMPLES_PER_CYCLE);

                    if (status == HAL_OK) {
                        current_state = ADC_STATE_BUSY;
                    }
                    else {
                    }
                }
                break;

            case ADC_STATE_BUSY:
                if (flags & DRV_ADC_FLAG_DMA_COMPLETE) {

                	HAL_ADC_Stop_DMA(&hadc1);
                	HAL_ADC_Stop(&hadc1);

                    for (size_t i = 0; i < active_channel_count; i++) {
                        uint8_t hw_ch = active_hw_channels[i];

                        uint16_t filtered = Filter_Algorithm_36Samples(&dma_buffer[i], active_channel_count);

                        if (hw_ch < MAX_HW_ADC_ID && callback_table[hw_ch].cb != NULL) {
                            callback_table[hw_ch].cb(hw_ch, (uint32_t)filtered, callback_table[hw_ch].ctx);
                        }
                    }

                    current_state = ADC_STATE_IDLE;
                }
                break;

            default:
                HAL_ADC_Stop_DMA(&hadc1);
                current_state = ADC_STATE_IDLE;
                break;
        }
    }
}

static void drvAdc_TimerCallback(void *arg) {
    if (drv_adc_thread_id) {
    	osThreadFlagsSet(drv_adc_thread_id, DRV_ADC_FLAG_TIMER_TRIG);
    }
}

void drvAdc_init(uint32_t enabledChannels_mask) {
    active_channel_count = 0;
    memset(callback_table, 0, sizeof(callback_table));
    memset(dma_buffer, 0, sizeof(dma_buffer));

    hadc1.Instance = ADC1;
    for (int i = 0; i < MAX_HW_ADC_ID; i++) {
        if (enabledChannels_mask & (1U << i)) {
        	active_channel_count++;
        }
    }

    hadc1.Init.NbrOfConversion = active_channel_count;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    HAL_ADC_Init(&hadc1);

    ADC_ChannelConfTypeDef sConfig = {0};
    uint8_t rank_counter = 1;
    active_channel_count = 0;

    for (int i = 0; i < MAX_HW_ADC_ID; i++) {
        if (enabledChannels_mask & (1U << i)) {
            active_hw_channels[active_channel_count++] = i;
            sConfig.Channel = i;
            sConfig.Rank = rank_counter++;
            sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
            HAL_ADC_ConfigChannel(&hadc1, &sConfig);

        }
    }

    const osThreadAttr_t task_attrs = {
    		.name = "DrvAdcTask",
			.stack_size = 512,
			.priority = osPriorityAboveNormal
    };

    drv_adc_thread_id = osThreadNew(drvAdc_WorkerTask, NULL, &task_attrs);

    const osTimerAttr_t timer_attrs = {
    		.name = "DrvAdcTimer"
    };

    drv_adc_timer_id = osTimerNew(drvAdc_TimerCallback, osTimerPeriodic, NULL, &timer_attrs);
}

void drvAdc_startMeasurement(void) {
    if (drv_adc_timer_id) {
    	osTimerStart(drv_adc_timer_id, MEASUREMENT_PERIOD_MS);
    }
}

void drvAdc_stopMeasurement(void) {
    if (drv_adc_timer_id) {
    	osTimerStop(drv_adc_timer_id);
    }
    HAL_ADC_Stop_DMA(&hadc1);
}

void drvAdc_registerCallback(uint8_t channel, drvAdc_Callback cb, void* ctx) {
    if (channel < MAX_HW_ADC_ID) {
        callback_table[channel].cb = cb;
        callback_table[channel].ctx = ctx;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        if (drv_adc_thread_id) {
        	osThreadFlagsSet(drv_adc_thread_id, DRV_ADC_FLAG_DMA_COMPLETE);
        }
    }
}
