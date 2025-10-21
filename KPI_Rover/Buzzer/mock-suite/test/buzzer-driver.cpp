#include <iostream>

#include <stdint.h>
#include <string.h>

#include <gtest/gtest.h>

extern "C" {
#include "main.h"
#include "scheduler.h"
#include "cmsis_os2.h"
//#include "test.h"

#include "driver.h"
}

#define LEN(a) sizeof(a) / sizeof(a[0])

#define PREP(...) \
	struct BuzzerObject bo; \
	struct StaticTimer_t tim; \
	memset(&bo, 0, sizeof(bo)); \
	memset(&tim, 0, sizeof(tim)); \
	system_ticks = 0; \
	gpio_init(); \
	timers_init(); \
	osTimerAttr_t attr = { \
		.name = NULL, \
		.attr_bits = 0, \
		.cb_mem = &tim, \
		.cb_size = sizeof(tim) \
	}; \
	osTimerNew(timer_cb, 1, &bo, &attr); \
	osTimerStart(&tim, 10)

void timer_cb(TimerHandle_t const tim_ptr)
{
	Buzzer_TimerTask((struct BuzzerObject *) tim_ptr->pvTimerID);
}

TEST(buzzer_driver, configure_correct)
{
	PREP();

	for (int i = 0x8000; i > 0; i >>= 1)
		EXPECT_EQ(Buzzer_ConfigurePort(&bo, GPIOD, i), 0) << i << " fails the configuration" << ::std::endl;
}

TEST(buzzer_driver, configure_wrong)
{
	PREP();

	uint16_t broken_values[] = {
		0xF000,
		0xFFFF,
		0x0000,
		0xA5A5,
		0xDEAD,
		0xBEEF
	};

	for (unsigned int i = 0; i < LEN(broken_values); i++)
		EXPECT_EQ(BUZZER_PIN_ERROR, Buzzer_ConfigurePort(&bo, GPIOD, broken_values[i])) << i << " does not cause pin error" << ::std::endl;
}

TEST(buzzer_driver, enable_correct)
{
	PREP();

	EXPECT_EQ(0, Buzzer_ConfigurePort(&bo, GPIOD, 0x8000));

	Buzzer_Enable(&bo);

	scheduler_run_for(15);

	EXPECT_EQ(0x8000, GPIOD->ODR);
}

TEST(buzzer_driver, enable_wrong)
{
	PREP();

	EXPECT_EQ(BUZZER_PIN_ERROR, Buzzer_ConfigurePort(&bo, GPIOD, 0xF000));

	Buzzer_Enable(&bo);

	scheduler_run_for(15);

	EXPECT_EQ(0x0000, GPIOD->ODR);
}

TEST(buzzer_driver, disable_correct)
{
	PREP();
	GPIOD->ODR = 0x0001;

	EXPECT_EQ(0, Buzzer_ConfigurePort(&bo, GPIOD, 0x0001));

	Buzzer_Disable(&bo);

	scheduler_run_for(15);

	EXPECT_EQ(0x0000, GPIOD->ODR);
}

TEST(buzzer_driver, disable_wrong)
{
	PREP();
	GPIOD->ODR = 0x0001;

	EXPECT_EQ(BUZZER_PIN_ERROR, Buzzer_ConfigurePort(&bo, GPIOD, 0x0101));

	Buzzer_Disable(&bo);

	scheduler_run_for(15);

	EXPECT_EQ(0x0001, GPIOD->ODR);
}


TEST(buzzer_driver, pulse_correct)
{
	PREP();

	EXPECT_EQ(0, Buzzer_ConfigurePort(&bo, GPIOD, 0x0001));

	EXPECT_EQ(0, Buzzer_Pulse(&bo, 200, 500, 2000));

	scheduler_run_for(15);

	EXPECT_EQ(0x0001, GPIOD->ODR);

	scheduler_run_for(210);

	EXPECT_EQ(0x0000, GPIOD->ODR);

	scheduler_run_for(310);

	EXPECT_EQ(0x0001, GPIOD->ODR);

	scheduler_run_for(210);

	EXPECT_EQ(0x0000, GPIOD->ODR);

	scheduler_run_for(1310);

	EXPECT_EQ(0x0000, GPIOD->ODR);
}

TEST(buzzer_driver, preemptive_pulse_stop)
{
	PREP();

	EXPECT_EQ(0, Buzzer_ConfigurePort(&bo, GPIOD, 0x0001));

	Buzzer_Pulse(&bo, 100, 200, 1000);

	scheduler_run_for(530);

	Buzzer_Disable(&bo);

	scheduler_run_for(15);

	EXPECT_EQ(0x0000, GPIOD->ODR);
}
