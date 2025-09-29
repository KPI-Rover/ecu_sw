################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../KPI_Rover/Logger/ul_ulog.c \
../KPI_Rover/Logger/ulog.c 

OBJS += \
./KPI_Rover/Logger/ul_ulog.o \
./KPI_Rover/Logger/ulog.o 

C_DEPS += \
./KPI_Rover/Logger/ul_ulog.d \
./KPI_Rover/Logger/ulog.d 


# Each subdirectory must supply rules for building sources it contributes
KPI_Rover/Logger/%.o KPI_Rover/Logger/%.su KPI_Rover/Logger/%.cyclo: ../KPI_Rover/Logger/%.c KPI_Rover/Logger/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DULOG_ENABLED -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../KPI_Rover/Logger -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Oz -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-KPI_Rover-2f-Logger

clean-KPI_Rover-2f-Logger:
	-$(RM) ./KPI_Rover/Logger/ul_ulog.cyclo ./KPI_Rover/Logger/ul_ulog.d ./KPI_Rover/Logger/ul_ulog.o ./KPI_Rover/Logger/ul_ulog.su ./KPI_Rover/Logger/ulog.cyclo ./KPI_Rover/Logger/ulog.d ./KPI_Rover/Logger/ulog.o ./KPI_Rover/Logger/ulog.su

.PHONY: clean-KPI_Rover-2f-Logger

