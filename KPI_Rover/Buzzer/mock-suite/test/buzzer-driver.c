#include <stdint.h>

#include "main.h"
#include "FreeRTOS.h"
#include "timers.h"

#include "scheduler.h"
#include "test.h"

#include "driver.h"

#define LEN(a) sizeof(a) / sizeof(a[0])

int driver_configure_correct(void)
{
	for (int i = 0x8000; i > 0; i >>= 1)
		if (Buzzer_ConfigurePort(GPIOD, i))
			return i;

	return 0;
}

int driver_configure_wrong(void)
{
	uint16_t broken_values[] = {
		0xF000,
		0xFFFF,
		0x0000,
		0xA5A5,
		0xDEAD,
		0xBEEF
	};

	for (int i = 0; i < LEN(broken_values); i++)
		if (BUZZER_PIN_ERROR != Buzzer_ConfigurePort(GPIOD, broken_values[i]))
			return i+1;

	return 0;
}

int driver_enable_correct(void)
{
	gpio_init();

	if (Buzzer_ConfigurePort(GPIOD, 0x8000))
		return 1;

	if (Buzzer_Enable())
		return 2;

	if (!(GPIOD->ODR & 0x8000))
		return 3;

	return 0;
}

int driver_enable_wrong(void)
{
	gpio_init();

	if (BUZZER_PIN_ERROR != Buzzer_ConfigurePort(GPIOD, 0xF000))
		return 1;

	if (BUZZER_NOT_INITIALIZED_ERROR != Buzzer_Enable())
		return 2;

	if (GPIOD->ODR & 0xF000)
		return 3;

	return 0;
}

int driver_disable_correct(void)
{
	gpio_init();
	GPIOD->ODR = 0x1;

	if (Buzzer_ConfigurePort(GPIOD, 0x0001))
		return 1;

	if (Buzzer_Disable())
		return 2;

	if (GPIOD->ODR & 0x0001)
		return 3;

	return 0;
}

int driver_disable_wrong(void)
{
	gpio_init();
	GPIOD->ODR = 0x1;

	if (BUZZER_PIN_ERROR != Buzzer_ConfigurePort(GPIOD, 0x0101))
		return 1;

	if (BUZZER_NOT_INITIALIZED_ERROR != Buzzer_Disable())
		return 2;

	if (!(GPIOD->ODR & 0x0001))
		return 3;

	return 0;
}

int driver_pulse_correct(void)
{
	gpio_init();
	timers_init();

	TimerHandle_t *timer_queue = get_timer_queue();

	if (Buzzer_ConfigurePort(GPIOD, 0x0001))
		return 1;

	if (Buzzer_Pulse(200, 500, 2000))
		return 2;

	if (!(GPIOD->ODR & 0x0001))
		return 3;

	scheduler_run_for(200);

	if (GPIOD->ODR & 0x0001)
		return 4;

	scheduler_run_for(300);

	if (!(GPIOD->ODR & 0x0001))
		return 5;

	scheduler_run_for(200);

	if (GPIOD->ODR & 0x0001)
		return 6;

	scheduler_run_for(1300);

	for (int i = 0; i < TIMER_QUEUE_SIZE; i++)
		if (timer_queue[i] != NULL && (timer_queue[i]->status | TIMER_ACTIVE))
			return 7;

	return 0;
}

int driver_preemptive_pulse_stop(void)
{
	gpio_init();
	timers_init();

	TimerHandle_t *timer_queue = get_timer_queue();

	if (Buzzer_ConfigurePort(GPIOD, 0x0001))
		return 1;

	if (Buzzer_Pulse(100, 200, 1000))
		return 2;

	scheduler_run_for(530);

	if (Buzzer_Disable())
		return 3;

	for (int i = 0; i < TIMER_QUEUE_SIZE; i++)
		if (timer_queue[i])
			return 4;

	return 0;
}

int dummy_broken_test(void)
{
	return Buzzer_ConfigurePort(GPIOD, 0xF000);
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
