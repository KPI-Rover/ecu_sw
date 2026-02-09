#include "PCA9685.h"
#include "stm32f4xx_hal.h"

/* I2C Handle defined in main.c */
extern I2C_HandleTypeDef hi2c1;

/* PCA9685 I2C address */
/* 0x40 is the 7-bit address. HAL requires it shifted left by 1 (0x80) */
#define PCA9685_ADDR     (0x40 << 1)

/* Register Addresses */
#define MODE1            0x00
#define PRESCALE         0xFE
#define LED0_ON_L        0x06

/**
 * Helper: Write a single byte to a register.
 */
static void pca9685_write(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1,
                      PCA9685_ADDR,
                      reg,
                      I2C_MEMADD_SIZE_8BIT,
                      &data,
                      1,
                      HAL_MAX_DELAY);
}

/**
 * Helper: Write 4 bytes consecutively (Burst Write).
 * Used to update all PWM registers (ON_L, ON_H, OFF_L, OFF_H) in one go.
 */
static void pca9685_write4(uint8_t reg, uint16_t on, uint16_t off)
{
    uint8_t buf[4];
    /* Split 12-bit values into Low and High bytes */
    buf[0] = on & 0xFF;       // ON_L
    buf[1] = on >> 8;         // ON_H
    buf[2] = off & 0xFF;      // OFF_L
    buf[3] = off >> 8;        // OFF_H

    HAL_I2C_Mem_Write(&hi2c1,
                      PCA9685_ADDR,
                      reg,
                      I2C_MEMADD_SIZE_8BIT,
                      buf,
                      4,
                      HAL_MAX_DELAY);
}

/**
 * Initialize the PCA9685 chip.
 * Setup frequency and enable auto-increment mode.
 */
void PCA9685_Init(void)
{
    /* 1. Reset device (Wake up) */
    pca9685_write(MODE1, 0x00);
    HAL_Delay(10);

    /* 2. Setup PWM Frequency ~1000Hz */
    /* Formula: prescale = 25MHz / (4096 * update_rate) - 1 */
    /* For 1000Hz: 25000000 / (4096 * 1000) - 1 ~= 5 */
    uint8_t prescale = 5;

    /* To change pre-scaler, we must be in SLEEP mode */
    pca9685_write(MODE1, 0x10); // Sleep mode
    HAL_Delay(1);

    pca9685_write(PRESCALE, prescale); // Write pre-scaler

    /* 3. Wake up and set Auto-Increment */
    /* 0xA1: 1010 0001
     * Bit 5 (0x20): Auto-Increment enabled (allows writing 4 bytes at once)
     * Bit 0 (0x01): ALLCALL enabled
     */
    pca9685_write(MODE1, 0xA1);
    HAL_Delay(5);
}

/**
 * Set PWM duty cycle for a specific channel.
 * @param channel: 0 to 15
 * @param on:  Step number to turn ON (usually 0)
 * @param off: Step number to turn OFF (0 to 4095) - This acts as Duty Cycle
 */
void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off)
{
    /* Safety check */
    if (channel > 15)
        return;

    /* 12-bit limit check (4095 is max) */
    if (off > 4095)
        off = 4095;

    /* Calculate register address for this channel */
    /* Each channel has 4 registers, starting at LED0_ON_L (0x06) */
    pca9685_write4(LED0_ON_L + 4 * channel, on, off);
}

void PCA9685_SetPin(uint8_t channel, uint8_t val)
{
    if (channel > 15) return;

    if (val) {
        // Full ON: Bit 4 в ON_H 1 - 4096, OFF в 0
        pca9685_write4(LED0_ON_L + 4 * channel, 0x1000, 0x0000);
    } else {
        // Full OFF: ON в 0, Bit 4 в OFF_H 1 - 4096
        pca9685_write4(LED0_ON_L + 4 * channel, 0x0000, 0x1000);
    }
}
