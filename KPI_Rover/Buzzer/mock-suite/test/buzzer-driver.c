#include <stdint.h>
#include <string.h>

#include "main.h"
#include "scheduler.h"
#include "cmsis_os2.h"
#include "test.h"

#include "driver.h"

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
	Buzzer_TimerTask(tim_ptr->pvTimerID);
}

int driver_configure_correct(void)
{
	PREP();

	for (int i = 0x8000; i > 0; i >>= 1)
		if (Buzzer_ConfigurePort(&bo, GPIOD, i))
			return i;

	return 0;
}

int driver_configure_wrong(void)
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

	for (int i = 0; i < LEN(broken_values); i++)
		if (BUZZER_PIN_ERROR != Buzzer_ConfigurePort(&bo, GPIOD, broken_values[i]))
			return i+1;

	return 0;
}

int driver_enable_correct(void)
{
	PREP();

	if (Buzzer_ConfigurePort(&bo, GPIOD, 0x8000))
		return 1;

	if (Buzzer_Enable(&bo))
		return 2;

	scheduler_run_for(15);

	if (!(GPIOD->ODR & 0x8000))
		return 3;

	return 0;
}

int driver_enable_wrong(void)
{
	PREP();

	if (BUZZER_PIN_ERROR != Buzzer_ConfigurePort(&bo, GPIOD, 0xF000))
		return 1;

	Buzzer_Enable(&bo);

	scheduler_run_for(15);

	if (GPIOD->ODR & 0xF000)
		return 2;

	return 0;
}

int driver_disable_correct(void)
{
	PREP();
	GPIOD->ODR = 0x1;

	if (Buzzer_ConfigurePort(&bo, GPIOD, 0x0001))
		return 1;

	if (Buzzer_Disable(&bo))
		return 2;

	scheduler_run_for(15);

	if (GPIOD->ODR & 0x0001)
		return 3;

	return 0;
}

int driver_disable_wrong(void)
{
	PREP();
	GPIOD->ODR = 0x1;

	if (BUZZER_PIN_ERROR != Buzzer_ConfigurePort(&bo, GPIOD, 0x0101))
		return 1;

	Buzzer_Disable(&bo);

	scheduler_run_for(15);

	if (!(GPIOD->ODR & 0x0001))
		return 2;

	return 0;
}


int driver_pulse_correct(void)
{
	PREP();

	if (Buzzer_ConfigurePort(&bo, GPIOD, 0x0001))
		return 1;

	if (Buzzer_Pulse(&bo, 200, 500, 2000))
		return 2;

	scheduler_run_for(15);

	if (!(GPIOD->ODR & 0x0001))
		return 3;

	scheduler_run_for(210);

	if (GPIOD->ODR & 0x0001)
		return 4;

	scheduler_run_for(310);

	if (!(GPIOD->ODR & 0x0001))
		return 5;

	scheduler_run_for(210);

	if (GPIOD->ODR & 0x0001)
		return 6;

	scheduler_run_for(1310);

	if (GPIOD->ODR & 0x0001)
		return 7;

	return 0;
}

int driver_preemptive_pulse_stop(void)
{
	PREP();

	if (Buzzer_ConfigurePort(&bo, GPIOD, 0x0001))
		return 1;

	if (Buzzer_Pulse(&bo, 100, 200, 1000))
		return 2;

	scheduler_run_for(530);

	if (Buzzer_Disable(&bo))
		return 3;

	scheduler_run_for(15);

	if (GPIOD->ODR & 1)
		return 4;

	return 0;
}

int dummy_broken_test(void)
{
	PREP();

	return Buzzer_ConfigurePort(&bo, GPIOD, 0xF000);
}

int main(void)
{
	INIT();

	TEST(driver_configure_correct);
	TEST(driver_configure_wrong);

	TEST(driver_enable_correct);
	TEST(driver_enable_wrong);

	TEST(driver_disable_correct);
	TEST(driver_disable_wrong);

	TEST(driver_pulse_correct);
	TEST(driver_preemptive_pulse_stop);

	TEST(dummy_broken_test);

	SUMMARY();
}
