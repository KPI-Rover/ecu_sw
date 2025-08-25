#include "KPIRover.h"
#include "cmsis_os.h"
#include "ADC/adc_manager.h"

void KPIRover_Init(void) {

	// ADC
	const osThreadAttr_t adcTask_attributes = {
	      .name = "adcTask",
	      .priority = (osPriority_t) osPriorityNormal,
	      .stack_size = 512 * 4
	  };
	(void) osThreadNew(ADC_Manager_Task, NULL, &adcTask_attributes);

}
