#include "PCA9685.h"
#include "stm32f4xx_hal.h"

/* ЗОВНІШНІЙ I2C */
extern I2C_HandleTypeDef hi2c1;

/* PCA9685 I2C address (0x40 default) */
#define PCA9685_ADDR     (0x40 << 1)

#define MODE1            0x00
#define PRESCALE         0xFE
#define LED0_ON_L        0x06

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

static void pca9685_write4(uint8_t reg, uint16_t on, uint16_t off)
{
    uint8_t buf[4];
    buf[0] = on & 0xFF;
    buf[1] = on >> 8;
    buf[2] = off & 0xFF;
    buf[3] = off >> 8;

    HAL_I2C_Mem_Write(&hi2c1,
                      PCA9685_ADDR,
                      reg,
                      I2C_MEMADD_SIZE_8BIT,
                      buf,
                      4,
                      HAL_MAX_DELAY);
}

void PCA9685_Init(void)
{
    pca9685_write(MODE1, 0x00);
    HAL_Delay(10);

    /* === Set frequency ~1 kHz === */
    uint8_t prescale = 5;   // ≈1017 Hz

    pca9685_write(MODE1, 0x10);
    HAL_Delay(1);

    pca9685_write(PRESCALE, prescale);

    pca9685_write(MODE1, 0xA1);
    HAL_Delay(5);
}

void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off)
{
    if (channel > 15)
        return;

    if (off > 4095)
        off = 4095;

    pca9685_write4(LED0_ON_L + 4 * channel, on, off);
}
