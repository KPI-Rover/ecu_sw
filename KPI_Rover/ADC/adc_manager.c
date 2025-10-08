#include "adc_manager.h"
#include "ulog.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "stdbool.h"

static const adc_channel_config_t adc_config[] = {
    { .channel = 2, .phys_high = 3.3f, .calibration_required = 1 }
};

#define ADC_CONFIG_COUNT (sizeof(adc_config) / sizeof(adc_config[0]))

#define ADC_TIMER_PERIOD_MS 100

static osTimerId_t adc_timer_handle;

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

static volatile bool adc_timer_flag = false;

static void adc_timer_callback(void *arg) {
    adc_timer_flag = true;
}


void ADC_Manager_Init(void) {
    uint8_t channels[ADC_CONFIG_COUNT];

    for (size_t i = 0; i < ADC_CONFIG_COUNT; i++) {
        channels[i] = adc_config[i].channel;
    }

    ADC_Driver_Init(channels, ADC_CONFIG_COUNT);

    ADC_Driver_RegisterCallback(adc_data_cb);

    ADC_Driver_Start();

    const osTimerAttr_t timer_attrs = {
			.name = "ADC_Timer"
	};

	adc_timer_handle = osTimerNew(adc_timer_callback, osTimerPeriodic, NULL, &timer_attrs);
	if (!adc_timer_handle) {
		ULOG_ERROR("Failed to create ADC timer");
		osThreadExit();
	}

	osDelay(10);

	if (osTimerStart(adc_timer_handle, ADC_TIMER_PERIOD_MS) != osOK) {
		ULOG_ERROR("Failed to start ADC timer");
		osThreadExit();
	}
}


void ADC_Manager_Task(void *argument) {
    ADC_Manager_Init();

    for (;;) {
    	ADC_Driver_ReadChannels();
    	if (adc_timer_flag) {
			adc_timer_flag = false;
			ADC_Driver_TimerTask();
		}
		osDelay(10);

//        int state = DB_ReadCalibState();
//
//		switch (0) { // (state)
//	        case 1: // start 0В
//	            ADC_Driver_StartCalibrationLow(adc_config[0].channel);
//		        DB_WriteCalibState(2);
//		        break;
//
//		    case 3: // start 3.3В
//		        ADC_Driver_StartCalibrationHigh(adc_config[0].channel);
//	            DB_WriteCalibState(4);
//		        break;
//
//			default:
//			    break;
//		}
    }
}
