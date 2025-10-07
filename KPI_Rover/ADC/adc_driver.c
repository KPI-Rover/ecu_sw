#include "adc_driver.h"
#include "stm32f4xx_hal.h"
#include "ulog.h"

#define MAX_ADC_CHANNELS 16
#define EMA_ALPHA 0.05f
#define CALIB_SAMPLES_TARGET 36

static uint8_t adc_channels[MAX_ADC_CHANNELS];
static uint16_t adc_raw_values[MAX_ADC_CHANNELS];
static uint16_t adc_filtered_values[MAX_ADC_CHANNELS];
static size_t channel_count = 0;

static adc_callback_t registered_callback = NULL;

static adc_calibration_t adc_calibration_data[MAX_ADC_CHANNELS];

static float adc_filtered_ema[MAX_ADC_CHANNELS] = {0.0f};

extern ADC_HandleTypeDef hadc1;

static volatile adc_driver_state_t adc_driver_state = ADC_STATE_IDLE;
static volatile uint32_t adc_state_change_tick = 0;
static volatile uint8_t adc_request_calib_low_channel = 0xFF;
static volatile uint8_t adc_request_calib_high_channel = 0xFF;

static adc_calib_substate_t calib_substate_low = CAL_SUB_IDLE;
static adc_calib_substate_t calib_substate_high = CAL_SUB_IDLE;

#define CALIB_STABLE_MS 10

static uint16_t calib_samples[MAX_ADC_CHANNELS][CALIB_SAMPLES_TARGET];
static uint8_t calib_sample_counts[MAX_ADC_CHANNELS] = {0};

static int find_channel_index(uint8_t channel) {
    for (size_t i = 0; i < channel_count; ++i) {
        if (adc_channels[i] == channel) return (int)i;
    }
    return -1;
}

void ADC_Driver_Init(const uint8_t* channels, size_t count) {
	channel_count = count > MAX_ADC_CHANNELS ? MAX_ADC_CHANNELS : count;
    for (size_t i = 0; i < channel_count; i++) {
    	adc_channels[i] = channels[i];
    	ADC_ChannelConfTypeDef sConfig = {0};
		sConfig.Channel = adc_channels[i];
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
			ULOG_ERROR("ADC: channel %u config failed", adc_channels[i]);
		} else {
			ULOG_INFO("ADC channel %u initialized", adc_channels[i]);
		}
    }
}

void ADC_Driver_Start(void) {
    if (channel_count == 0) {
        ULOG_ERROR("ADC start failed: no channels configured");
        return;
    }
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        ULOG_ERROR("Failed to start ADC (polling mode)");
    } else {
        ULOG_INFO("ADC started in polling mode with %d channels", channel_count);
    }
}

void ADC_Driver_StartCalibrationLow(uint8_t channel) {
	adc_request_calib_low_channel = channel;
	int idx = find_channel_index(channel);
	if (idx >= 0) calib_sample_counts[idx] = 0;

	adc_driver_state = ADC_STATE_CALIB_LOW_PENDING;
	calib_substate_low = CAL_SUB_WAIT_STABLE;
	adc_state_change_tick = HAL_GetTick();
	ULOG_INFO("ADC: queued START CALIB LOW for channel %u", channel);
}

void ADC_Driver_StartCalibrationHigh(uint8_t channel) {
	adc_request_calib_high_channel = channel;
	int idx = find_channel_index(channel);
	if (idx >= 0) calib_sample_counts[idx] = 0;

    adc_driver_state = ADC_STATE_CALIB_HIGH_PENDING;
    calib_substate_high = CAL_SUB_WAIT_STABLE;
	adc_state_change_tick = HAL_GetTick();
	ULOG_INFO("ADC: queued START CALIB HIGH for channel %u", channel);
}



void ADC_Driver_SetCalibration(uint8_t channel, adc_calibration_t calib) {
	 int idx = find_channel_index(channel);
	 if (idx >= 0) {
	     adc_calibration_data[idx] = calib;
	     ULOG_INFO("Calibration set for channel %u", channel);
	 } else {
	     ULOG_WARNING("Attempted to calibrate unknown channel %u", channel);
	 }
}

float ADC_Driver_GetCalibratedValue(uint8_t channel) {
    int idx = find_channel_index(channel);
    if (idx < 0) {
        ULOG_WARNING("Calibrated value requested for unknown channel %u", channel);
        return 0.0f;
    }

    adc_calibration_t calib = adc_calibration_data[idx];
    uint16_t raw = adc_filtered_values[idx];

    if (calib.raw_high == calib.raw_low) return 0.0f;

    float phys = calib.phys_low + ((float)(raw - calib.raw_low)) *
                 (calib.phys_high - calib.phys_low) /
                 (float)(calib.raw_high - calib.raw_low);
    return phys;
}

void ADC_Driver_RegisterCallback(adc_callback_t cb) {
    registered_callback = cb;
    ULOG_DEBUG("ADC callback registered");
}

uint16_t ADC_Driver_GetLastValue(uint8_t channel) {
    int idx = find_channel_index(channel);
    if (idx >= 0) {
        return adc_filtered_values[idx];
    }
    ULOG_WARNING("ADC: Requested value for unregistered channel %u", channel);
    return 0;
}


void ADC_Driver_ReadChannels(void) {
    for (size_t i = 0; i < channel_count; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) {
            uint16_t raw = HAL_ADC_GetValue(&hadc1);

            adc_filtered_ema[i] = EMA_ALPHA * (float)raw +
                                  (1.0f - EMA_ALPHA) * adc_filtered_ema[i];
            adc_filtered_values[i] = (uint16_t)adc_filtered_ema[i];

            if (adc_driver_state == ADC_STATE_CALIB_LOW_PENDING &&
				calib_substate_low == CAL_SUB_COLLECT &&
				adc_channels[i] == adc_request_calib_low_channel)
			{
				int idx = find_channel_index(adc_channels[i]);
				if (idx >= 0 && calib_sample_counts[idx] < CALIB_SAMPLES_TARGET) {
					calib_samples[idx][calib_sample_counts[idx]++] = adc_filtered_values[idx];
				}
			}

			if (adc_driver_state == ADC_STATE_CALIB_HIGH_PENDING &&
				calib_substate_high == CAL_SUB_COLLECT &&
				adc_channels[i] == adc_request_calib_high_channel)
			{
				int idx = find_channel_index(adc_channels[i]);
				if (idx >= 0 && calib_sample_counts[idx] < CALIB_SAMPLES_TARGET) {
					calib_samples[idx][calib_sample_counts[idx]++] = adc_filtered_values[idx];
				}
			}

            if (registered_callback) {
                registered_callback(adc_channels[i], adc_filtered_values[i]);
            }
        }
        HAL_ADC_Stop(&hadc1);
    }
}



void ADC_Driver_TimerTask(void) {
    uint32_t now = HAL_GetTick();
    static adc_measurements_t adc_meas = {0};

    switch (adc_driver_state) {

        case ADC_STATE_IDLE: {
            if (now >= adc_meas.next_voltage_tick) {
                adc_driver_state = ADC_STATE_MEASURE_VOLTAGE;
            }
        } break;

        case ADC_STATE_MEASURE_VOLTAGE: {
            int idx = find_channel_index(2);
            if (idx >= 0) {
				adc_meas.voltage_raw = ADC_Driver_GetLastValue(adc_channels[idx]);
				adc_meas.voltage_cal = ADC_Driver_GetCalibratedValue(adc_channels[idx]);
				ULOG_INFO("Voltage: raw=%u, filtered=%.3f V",
						  adc_meas.voltage_raw,
						  adc_meas.voltage_cal);
			}
            adc_meas.next_voltage_tick = now + 10;
            adc_driver_state = ADC_STATE_IDLE;
        } break;

        case ADC_STATE_REQUEST_CALIB_LOW: {
            uint8_t ch = adc_request_calib_low_channel;
            int idx = find_channel_index(ch);
            if (idx >= 0) {
                calib_sample_counts[idx] = 0;
                adc_driver_state = ADC_STATE_CALIB_LOW_PENDING;
                adc_state_change_tick = now;
                ULOG_INFO("ADC: CALIB LOW pending (channel %u)", ch);
            } else {
                adc_driver_state = ADC_STATE_IDLE;
            }
        } break;

        case ADC_STATE_CALIB_LOW_PENDING: {
            uint8_t ch = adc_request_calib_low_channel;
            int idx = find_channel_index(ch);
            if (idx < 0) {
                adc_driver_state = ADC_STATE_IDLE;
                break;
            }

            switch (calib_substate_low) {
                case CAL_SUB_WAIT_STABLE:
                    if ((now - adc_state_change_tick) > CALIB_STABLE_MS) {
                        calib_sample_counts[idx] = 0;
                        calib_substate_low = CAL_SUB_COLLECT;
                        ULOG_INFO("ADC: CALIB LOW COLLECT start (channel %u)", ch);
                    }
                    break;

                case CAL_SUB_COLLECT:
                    if (calib_sample_counts[idx] >= CALIB_SAMPLES_TARGET) {
                        calib_substate_low = CAL_SUB_COMPUTE;
                    }
                    break;

                case CAL_SUB_COMPUTE: {
                    uint16_t min1 = 0xFFFF, min2 = 0xFFFF, max1 = 0, max2 = 0;
                    uint32_t sum = 0;
                    for (size_t i = 0; i < CALIB_SAMPLES_TARGET; i++) {
                        uint16_t v = calib_samples[idx][i];
                        sum += v;
                        if (v < min1) { min2 = min1; min1 = v; }
                        else if (v < min2) min2 = v;
                        if (v > max1) { max2 = max1; max1 = v; }
                        else if (v > max2) max2 = v;
                    }
                    uint16_t avg_min = (min1 + min2) / 2;
                    uint16_t avg_max = (max1 + max2) / 2;
                    uint16_t avg_all = sum / CALIB_SAMPLES_TARGET;
                    uint16_t final = (avg_min + avg_max + avg_all) / 3;

                    adc_calibration_t calib = adc_calibration_data[idx];
                    calib.raw_low = final;
                    calib.phys_low = 0.0f;
                    ADC_Driver_SetCalibration(ch, calib);

                    ULOG_INFO("ADC: CALIB LOW done ch=%u raw=%u", ch, final);
                    adc_driver_state = ADC_STATE_IDLE;
                    adc_request_calib_low_channel = 0xFF;
                    calib_substate_low = CAL_SUB_IDLE;
                } break;

                default:
                    calib_substate_low = CAL_SUB_IDLE;
                    adc_driver_state = ADC_STATE_IDLE;
                    break;
            }
        } break;

        case ADC_STATE_REQUEST_CALIB_HIGH: {
            uint8_t ch = adc_request_calib_high_channel;
            int idx = find_channel_index(ch);
            if (idx >= 0) {
                calib_sample_counts[idx] = 0;
                adc_driver_state = ADC_STATE_CALIB_HIGH_PENDING;
                adc_state_change_tick = now;
                ULOG_INFO("ADC: CALIB HIGH pending (channel %u)", ch);
            } else {
                adc_driver_state = ADC_STATE_IDLE;
            }
        } break;

        case ADC_STATE_CALIB_HIGH_PENDING: {
            uint8_t ch = adc_request_calib_high_channel;
            int idx = find_channel_index(ch);
            if (idx < 0) {
                adc_driver_state = ADC_STATE_IDLE;
                break;
            }

            switch (calib_substate_high) {
                case CAL_SUB_WAIT_STABLE:
                    if ((now - adc_state_change_tick) > CALIB_STABLE_MS) {
                        calib_sample_counts[idx] = 0;
                        calib_substate_high = CAL_SUB_COLLECT;
                        ULOG_INFO("ADC: CALIB HIGH COLLECT start (channel %u)", ch);
                    }
                    break;

                case CAL_SUB_COLLECT:
                    if (calib_sample_counts[idx] >= CALIB_SAMPLES_TARGET) {
                        calib_substate_high = CAL_SUB_COMPUTE;
                    }
                    break;

                case CAL_SUB_COMPUTE: {
                    uint16_t min1 = 0xFFFF, min2 = 0xFFFF, max1 = 0, max2 = 0;
                    uint32_t sum = 0;
                    for (size_t i = 0; i < CALIB_SAMPLES_TARGET; i++) {
                        uint16_t v = calib_samples[idx][i];
                        sum += v;
                        if (v < min1) { min2 = min1; min1 = v; }
                        else if (v < min2) min2 = v;
                        if (v > max1) { max2 = max1; max1 = v; }
                        else if (v > max2) max2 = v;
                    }
                    uint16_t avg_min = (min1 + min2) / 2;
                    uint16_t avg_max = (max1 + max2) / 2;
                    uint16_t avg_all = sum / CALIB_SAMPLES_TARGET;
                    uint16_t final = (avg_min + avg_max + avg_all) / 3;

                    adc_calibration_t calib = adc_calibration_data[idx];
                    calib.raw_high = final;
                    calib.phys_high = 3.3f;
                    ADC_Driver_SetCalibration(ch, calib);

                    ULOG_INFO("ADC: CALIB HIGH done ch=%u raw=%u", ch, final);
                    adc_driver_state = ADC_STATE_IDLE;
                    adc_request_calib_high_channel = 0xFF;
                    calib_substate_high = CAL_SUB_IDLE;
                } break;

                default:
                    calib_substate_high = CAL_SUB_IDLE;
                    adc_driver_state = ADC_STATE_IDLE;
                    break;
            }
        } break;

        case ADC_STATE_ERROR:
            ULOG_ERROR("ADC driver error state");
            break;

        default:
        	 ULOG_INFO("ADC: unknown state=%u", adc_driver_state);
        	    adc_driver_state = ADC_STATE_IDLE;
            break;
    }
}
