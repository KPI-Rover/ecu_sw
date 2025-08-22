#include <stdint.h>

#include "main.h"
#include "FreeRTOS.h"
#include "timers.h"

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
		0xA5A5
	};

	for (int i = 0; i < LEN(broken_values); i++)
		if (!Buzzer_ConfigurePort(GPIOD, broken_values[i]))
			return i+1;

	return 0;
}

int dummy_broken_test(void)
{
	return Buzzer_ConfigurePort(GPIOD, 0xF000);
}

int main(void)
{
	TEST(driver_configure_correct);
	TEST(driver_configure_wrong);
	TEST(dummy_broken_test);
}
