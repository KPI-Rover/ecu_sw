#include "adc_manager.h"
#include "ulog.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

static const adc_channel_config_t adc_config[] = {
    { .channel = 2, .phys_high = 3.3f, .calibration_required = 1 }
};

static uint32_t last_log_tick = 0;

#define ADC_CONFIG_COUNT (sizeof(adc_config) / sizeof(adc_config[0]))
#define ADC_TASK_FLAG_TIMER   (1U << 0)

#define ADC_TIMER_PERIOD_MS 100

static osTimerId_t adc_periodic_timer = NULL;
static osThreadId_t adcTaskHandle = NULL;

static uint16_t adc_last_raw[ADC_CONFIG_COUNT];
static float    adc_last_cal[ADC_CONFIG_COUNT];

static void adc_data_cb(uint8_t channel, uint16_t value) {
    for (size_t i = 0; i < ADC_CONFIG_COUNT; i++) {
        if (adc_config[i].channel == channel) {
            adc_last_raw[i] = value;
            adc_last_cal[i] = ADC_Driver_GetCalibratedValue(channel);
            break;
        }
    }
}

static void adc_periodic_timer_cb(void *arg) {
	ULOG_INFO("adc_periodic_timer_cb");
    (void)arg;
    osThreadFlagsSet(adcTaskHandle, ADC_TASK_FLAG_TIMER);
}

void ADC_Manager_Init(void) {
    uint8_t channels[ADC_CONFIG_COUNT];

    for (size_t i = 0; i < ADC_CONFIG_COUNT; i++) {
        channels[i] = adc_config[i].channel;
    }

    ADC_Driver_Init(channels, ADC_CONFIG_COUNT);

    ADC_Driver_Start();

    ADC_Driver_RegisterCallback(adc_data_cb);

    adcTaskHandle = osThreadGetId();

    const osTimerAttr_t timer_attr = {
      .name = "ADC_Periodic",
    };
    adc_periodic_timer = osTimerNew(adc_periodic_timer_cb, osTimerPeriodic, NULL, &timer_attr);
}


void ADC_Manager_Task(void *argument) {

	osTimerStart(adc_periodic_timer, ADC_TIMER_PERIOD_MS);

	ADC_Manager_Init();

    for (;;) {

    	 uint32_t flags = osThreadFlagsWait(ADC_TASK_FLAG_TIMER, osFlagsWaitAny, 10);

    	if (flags & ADC_TASK_FLAG_TIMER) {
    	    ADC_Driver_TimerTask();
    	}

    	uint32_t now = HAL_GetTick();
		if (now - last_log_tick >= 500) {
			last_log_tick = now;
			for (size_t i = 0; i < ADC_CONFIG_COUNT; ++i) {
				int cal_mv = (int)(adc_last_cal[i] * 1000.0f);
				ULOG_INFO("ADC CH%u: filtered=%u cal=%d mV",
				adc_config[i].channel, adc_last_raw[i], cal_mv);
			}
		}

//      int state = DB_ReadCalibState();

      switch (0) { // (state)
//        case 1: // start 0В
//          ADC_Driver_StartCalibrationLow(adc_config[0].channel);
//              DB_WriteCalibState(2);
//              break;
//
//          case 3: // start 3.3В
//            ADC_Driver_StartCalibrationHigh(adc_config[0].channel);
//              DB_WriteCalibState(4);
//              break;

         default:
           break;
      }
      osDelay(100);
    }
}
