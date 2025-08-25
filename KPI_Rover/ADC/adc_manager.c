#include "adc_manager.h"
#include "ulog.h"
#include "cmsis_os.h"

static const adc_channel_config_t adc_config[] = {
    { .channel = 2, .phys_high = 3.3f, .calibration_required = 1 }
};

static uint32_t last_log_tick = 0;

#define ADC_CONFIG_COUNT (sizeof(adc_config) / sizeof(adc_config[0]))

static void adc_manager_callback(uint8_t channel, uint16_t value) {
    uint32_t now = HAL_GetTick();

    if (now - last_log_tick >= 500) {
    	int filtered = ADC_Driver_GetLastValue(channel);
        float calibrated = ADC_Driver_GetCalibratedValue(channel);
        int calibrated_mv = (int)(calibrated * 1000);
        ULOG_INFO("CH%u: RAW=%u, CALIB=%d mV", channel, filtered, calibrated_mv);
        last_log_tick = now;
    }
}

void ADC_Manager_Init(void) {
    uint8_t channels[ADC_CONFIG_COUNT];

    for (size_t i = 0; i < ADC_CONFIG_COUNT; i++) {
        channels[i] = adc_config[i].channel;
    }

    ADC_Driver_Init(channels, ADC_CONFIG_COUNT);

    ADC_Driver_Start();

    ADC_Driver_RegisterCallback(adc_manager_callback);


}

void ADC_Manager_Task(void *argument) {
    ADC_Manager_Init();

    for (;;) {
//    	int state = DB_ReadCalibState();

    	switch (0) { // (state)
//    		case 1: // start 0В
//    			ADC_Driver_StartCalibrationLow(adc_config[0].channel);
//    	        DB_WriteCalibState(2);
//    	        break;
//
//    	    case 3: // start 3.3В
//    	    	ADC_Driver_StartCalibrationHigh(adc_config[0].channel);
//    	        DB_WriteCalibState(4);
//    	        break;

    	   default:
    		   break;
    	}

    	osDelay(200);
    }
}
