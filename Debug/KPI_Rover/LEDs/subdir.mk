################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../KPI_Rover/LEDs/driver.c \
../KPI_Rover/LEDs/manager.c 

OBJS += \
./KPI_Rover/LEDs/driver.o \
./KPI_Rover/LEDs/manager.o 

C_DEPS += \
./KPI_Rover/LEDs/driver.d \
./KPI_Rover/LEDs/manager.d 


# Each subdirectory must supply rules for building sources it contributes
KPI_Rover/LEDs/%.o KPI_Rover/LEDs/%.su KPI_Rover/LEDs/%.cyclo: ../KPI_Rover/LEDs/%.c KPI_Rover/LEDs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DULOG_ENABLED -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../KPI_Rover/Logger -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Oz -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-KPI_Rover-2f-LEDs

clean-KPI_Rover-2f-LEDs:
	-$(RM) ./KPI_Rover/LEDs/driver.cyclo ./KPI_Rover/LEDs/driver.d ./KPI_Rover/LEDs/driver.o ./KPI_Rover/LEDs/driver.su ./KPI_Rover/LEDs/manager.cyclo ./KPI_Rover/LEDs/manager.d ./KPI_Rover/LEDs/manager.o ./KPI_Rover/LEDs/manager.su

.PHONY: clean-KPI_Rover-2f-LEDs

