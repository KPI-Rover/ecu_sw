#include "ulAdc.h"
#include "drvAdc.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include "ulog.h"
#include "Database/ulDatabase.h"

#define HW_CH_BATTERY      11
#define HW_CH_TEMP         16

#define ADC_MAX_VAL        4095.0f
#define ADC_VREF           3.3f

// 1. Battery: Voltage Divider (100k + 22k) / 22k
#define BAT_DEFAULT_SCALE  ((ADC_VREF / ADC_MAX_VAL) * ((100.0f + 22.0f) / 22.0f))
#define BAT_DEFAULT_OFFSET 0

// 2. Temp: T = (V - V25)/Slope + 25
// V = Raw * (3.3/4095).
// T = [Raw * (3.3/4095) - V25] / Slope + 25
// T = Raw * [3.3 / (4095*Slope)] - [V25/Slope - 25]
// T = Scale * (Raw - Offset_Raw)
// if Scale = 3.3 / (4095*Slope) and Offset_Raw represents the zero-crossing.
#define TEMP_V25           0.76f
#define TEMP_SLOPE         0.0025f
#define TEMP_DEFAULT_SCALE (ADC_VREF / (ADC_MAX_VAL * TEMP_SLOPE))

#define TEMP_DEFAULT_OFFSET (int32_t)(((TEMP_V25 - (25.0f * TEMP_SLOPE)) * ADC_MAX_VAL) / ADC_VREF)

#define ADC_CAL_STATUS_OK           0x00
#define ADC_CAL_STATUS_IN_PROGRESS  0x01
#define ADC_CAL_STATUS_ERROR        0xFF

#define LOG_INTERVAL_MS 1000

typedef enum {
    CAL_STATE_IDLE = 0,
    CAL_STATE_WAIT_FOR_SAMPLE
} adc_cal_state_t;

typedef struct {
    uint8_t hw_channel;

    uint16_t db_id_val;
    uint16_t db_id_cal_offset;
    uint16_t db_id_cal_scale;

    volatile int32_t active_offset;
    volatile float   active_scale;

    adc_cal_state_t cal_state;
    uint8_t         pending_point;
    float           pending_target;

    volatile uint32_t last_raw;
    volatile float    last_phys;
} adc_channel_ctx_t;

adc_channel_ctx_t app_adc_ctx[] = {
    {
        .hw_channel = HW_CH_BATTERY,
        .db_id_val = PARAM_BATTERY_VOLTAGE,
        .db_id_cal_offset = ADC_CAL_CH_11_OFFSET,
        .db_id_cal_scale  = ADC_CAL_CH_11_SCALE,
        .active_offset = BAT_DEFAULT_OFFSET,
        .active_scale  = BAT_DEFAULT_SCALE,
		.cal_state = CAL_STATE_IDLE
    },
    {
        .hw_channel = HW_CH_TEMP,
        .db_id_val = PARAM_MCU_TEMPERATURE,
        .db_id_cal_offset = ADC_CAL_CH_TEMP_OFFSET,
        .db_id_cal_scale  = ADC_CAL_CH_TEMP_SCALE,
        .active_offset = TEMP_DEFAULT_OFFSET,
        .active_scale  = TEMP_DEFAULT_SCALE,
		.cal_state = CAL_STATE_IDLE
    }
};

#define CTX_COUNT (sizeof(app_adc_ctx)/sizeof(app_adc_ctx[0]))

static void ulAdc_GetHardcodedDefaults_Bat(float* defScale, int32_t* defOffset) {
    const float Vref = 3.3f;
    const float MaxAdc = 4095.0f;
    const float DivFactor = (100.0f + 22.0f) / 22.0f; // (R1+R2)/R2

    // Val = Scale * (Raw - Offset)
    // Offset = 0
    *defOffset = 0;
    // Scale = (Vref / MaxAdc) * DivFactor
    *defScale = (Vref / MaxAdc) * DivFactor;
}

static void ulAdc_GetHardcodedDefaults_Temp(float* defScale, int32_t* defOffset) {
    const float Vref = 3.3f;
    const float MaxAdc = 4095.0f;
    const float V25 = 0.76f;
    const float Slope = 0.0025f;

    // T = (Raw * (Vref/Max) - V25) / Slope + 25
    // T = (Raw * Vref/Max)/Slope - V25/Slope + 25
    // T = Raw * (Vref/(Max*Slope)) - (V25/Slope - 25)
    // T = Scale * (Raw - Offset_Raw)

    float scale_val = Vref / (MaxAdc * Slope);
    float offset_val_phys = (V25 / Slope) - 25.0f;

    // Offset_Raw = offset_val_phys / scale_val

    *defScale = scale_val;
    *defOffset = (int32_t)(offset_val_phys / scale_val);
}

float ulAdc_applyCalibration(uint8_t channel, uint32_t rawValue) {
    adc_channel_ctx_t* ctx = NULL;
    for(size_t i=0; i<CTX_COUNT; i++) {
        if(app_adc_ctx[i].hw_channel == channel) {
            ctx = &app_adc_ctx[i];
            break;
        }
    }

    if (!ctx) {
    	return 0.0f;
    }

    // Y = Scale * (X - Offset)
    int32_t raw_shifted = (int32_t)rawValue - ctx->active_offset;
    return (float)raw_shifted * ctx->active_scale;
}

void ulAdc_CallbackHandler(uint8_t channel, uint32_t filteredRaw, void* ctx_void) {
    adc_channel_ctx_t* ctx = (adc_channel_ctx_t*)ctx_void;
    if (!ctx) {
    	return;
    }
    ctx->last_raw = filteredRaw;

    switch (ctx->cal_state) {

        case CAL_STATE_IDLE:
            {
                uint8_t start_cmd = 0;

                if (ulDatabase_getUint8(ADC_CALIBRATION_START, &start_cmd) && start_cmd == 0xF1) {
                    uint8_t target_ch = 0;
                    ulDatabase_getUint8(ADC_CALIBRATION_CHANNEL_ID, &target_ch);
                    if (target_ch == ctx->hw_channel) {
                        ulDatabase_getUint8(ADC_CALIBRATION_POINT, &ctx->pending_point);
                        ulDatabase_getFloat(ADC_CALIBRATION_POINT_VALUE, &ctx->pending_target);

                        ctx->cal_state = CAL_STATE_WAIT_FOR_SAMPLE;
                    }
                }
            }
            break;

        case CAL_STATE_WAIT_FOR_SAMPLE:
			{
				bool cal_ok = true;

				if (ctx->pending_point == 0) {
					//OFFSET CALIBRATION ---
					ctx->active_offset = (int32_t)filteredRaw;
					ulDatabase_setInt32(ctx->db_id_cal_offset, ctx->active_offset);
				}
				else if (ctx->pending_point == 1) {
					//SCALE CALIBRATION ---
					int32_t raw_shifted = (int32_t)filteredRaw - ctx->active_offset;

					if (abs(raw_shifted) > 10) {
						ctx->active_scale = ctx->pending_target / (float)raw_shifted;
						ulDatabase_setFloat(ctx->db_id_cal_scale, ctx->active_scale);
					} else {
						cal_ok = false;
					}
				}
				else {
					cal_ok = false;
				}

				if (cal_ok) {
					ulDatabase_setUint8(ADC_CALIBRATION_START, ADC_CAL_STATUS_OK);
				} else {
					ulDatabase_setUint8(ADC_CALIBRATION_START, ADC_CAL_STATUS_ERROR);
				}
				ctx->cal_state = CAL_STATE_IDLE;
			}
			break;

        default:
            ctx->cal_state = CAL_STATE_IDLE;
            break;
    }

    float phys_val = ulAdc_applyCalibration(channel, filteredRaw);

    ctx->last_phys = phys_val;
    ulDatabase_setFloat(ctx->db_id_val, phys_val);
}

void ulAdc_init(void) {
    uint32_t adc_mask = 0;

    for (size_t i = 0; i < CTX_COUNT; i++) {
    	float def_scale;
		int32_t def_offset;

		if (app_adc_ctx[i].hw_channel == HW_CH_BATTERY) {
			ulAdc_GetHardcodedDefaults_Bat(&def_scale, &def_offset);
		} else {
			ulAdc_GetHardcodedDefaults_Temp(&def_scale, &def_offset);
		}

        adc_channel_ctx_t* ctx = &app_adc_ctx[i];
        adc_mask |= (1U << ctx->hw_channel);

        int32_t stored_offset;
		float stored_scale;

		if (ulDatabase_getInt32(ctx->db_id_cal_offset, &stored_offset) && stored_offset != 0) {
		    ctx->active_offset = stored_offset;
		} else {
		    ctx->active_offset = def_offset;
		}

		if (ulDatabase_getFloat(ctx->db_id_cal_scale, &stored_scale) &&
			isfinite(stored_scale) && fabsf(stored_scale) > 1e-9f) {
			ctx->active_scale = stored_scale;
		} else {
			ctx->active_scale = def_scale;
		}
    }

    drvAdc_init(adc_mask);

    for (size_t i = 0; i < CTX_COUNT; i++) {
        drvAdc_registerCallback(app_adc_ctx[i].hw_channel, ulAdc_CallbackHandler, (void*)&app_adc_ctx[i]);
    }

    drvAdc_startMeasurement();
}

void ulAdc_task(void *argument) {
    ulAdc_init();

    for (;;) {
        osDelay(LOG_INTERVAL_MS);

        for (int i = 0; i < CTX_COUNT; i++) {
            adc_channel_ctx_t* ctx = &app_adc_ctx[i];

            ULOG_INFO("[ADC CH%d] Raw: %lu | Phys: %.3f", ctx->hw_channel, ctx->last_raw, ctx->last_phys);
            osDelay(20);
        }
    }
}
