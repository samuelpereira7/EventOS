################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/kernel/portable/device/NXP/LPC17xx/port.c 

OBJS += \
./Kernel/Port/LPC17xx/port.o 

C_DEPS += \
./Kernel/Port/LPC17xx/port.d 


# Each subdirectory must supply rules for building sources it contributes
Kernel/Port/LPC17xx/port.o: D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/kernel/portable/device/NXP/LPC17xx/port.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC17xx -DLPCXPRESSO_LPC17xx -I../../../../inc -I../../../../../../drivers/LPC17xx_HAL_Drivers/inc -I../../../../../../drivers/cmsis/include -I../../../../../../drivers/bsp/EA-XPR-021 -I../../../../../../drivers/cmsis/device/NXP/LPC17xx/include -I../../../../../../kernel/config -I../../../../../../kernel/core -I../../../../../../kernel/portable/device/NXP/LPC17xx -I../../../../../../middlewares/Log -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


