################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_clkpwr.c \
D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_i2c.c \
D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_libcfg_default.c \
D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_pinsel.c \
D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_rtc.c \
D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_uart.c 

OBJS += \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_clkpwr.o \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_i2c.o \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_libcfg_default.o \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_pinsel.o \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_rtc.o \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_uart.o 

C_DEPS += \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_clkpwr.d \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_i2c.d \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_libcfg_default.d \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_pinsel.d \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_rtc.d \
./Drivers/LPC17xx_HAL_Drivers/lpc17xx_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/LPC17xx_HAL_Drivers/lpc17xx_clkpwr.o: D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_clkpwr.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC17xx -DLPCXPRESSO_LPC17xx -I../../../../inc -I../../../../../../drivers/LPC17xx_HAL_Drivers/inc -I../../../../../../drivers/cmsis/include -I../../../../../../drivers/bsp/EA-XPR-021 -I../../../../../../drivers/cmsis/device/NXP/LPC17xx/include -I../../../../../../kernel/config -I../../../../../../kernel/core -I../../../../../../kernel/portable/device/NXP/LPC17xx -I../../../../../../middlewares/Log -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Drivers/LPC17xx_HAL_Drivers/lpc17xx_i2c.o: D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_i2c.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC17xx -DLPCXPRESSO_LPC17xx -I../../../../inc -I../../../../../../drivers/LPC17xx_HAL_Drivers/inc -I../../../../../../drivers/cmsis/include -I../../../../../../drivers/bsp/EA-XPR-021 -I../../../../../../drivers/cmsis/device/NXP/LPC17xx/include -I../../../../../../kernel/config -I../../../../../../kernel/core -I../../../../../../kernel/portable/device/NXP/LPC17xx -I../../../../../../middlewares/Log -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Drivers/LPC17xx_HAL_Drivers/lpc17xx_libcfg_default.o: D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_libcfg_default.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC17xx -DLPCXPRESSO_LPC17xx -I../../../../inc -I../../../../../../drivers/LPC17xx_HAL_Drivers/inc -I../../../../../../drivers/cmsis/include -I../../../../../../drivers/bsp/EA-XPR-021 -I../../../../../../drivers/cmsis/device/NXP/LPC17xx/include -I../../../../../../kernel/config -I../../../../../../kernel/core -I../../../../../../kernel/portable/device/NXP/LPC17xx -I../../../../../../middlewares/Log -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Drivers/LPC17xx_HAL_Drivers/lpc17xx_pinsel.o: D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_pinsel.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC17xx -DLPCXPRESSO_LPC17xx -I../../../../inc -I../../../../../../drivers/LPC17xx_HAL_Drivers/inc -I../../../../../../drivers/cmsis/include -I../../../../../../drivers/bsp/EA-XPR-021 -I../../../../../../drivers/cmsis/device/NXP/LPC17xx/include -I../../../../../../kernel/config -I../../../../../../kernel/core -I../../../../../../kernel/portable/device/NXP/LPC17xx -I../../../../../../middlewares/Log -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Drivers/LPC17xx_HAL_Drivers/lpc17xx_rtc.o: D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_rtc.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC17xx -DLPCXPRESSO_LPC17xx -I../../../../inc -I../../../../../../drivers/LPC17xx_HAL_Drivers/inc -I../../../../../../drivers/cmsis/include -I../../../../../../drivers/bsp/EA-XPR-021 -I../../../../../../drivers/cmsis/device/NXP/LPC17xx/include -I../../../../../../kernel/config -I../../../../../../kernel/core -I../../../../../../kernel/portable/device/NXP/LPC17xx -I../../../../../../middlewares/Log -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Drivers/LPC17xx_HAL_Drivers/lpc17xx_uart.o: D:/JoaoDoAviao/Inatel/TCC/repositorio/EventOS/drivers/LPC17xx_HAL_Drivers/src/lpc17xx_uart.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC17xx -DLPCXPRESSO_LPC17xx -I../../../../inc -I../../../../../../drivers/LPC17xx_HAL_Drivers/inc -I../../../../../../drivers/cmsis/include -I../../../../../../drivers/bsp/EA-XPR-021 -I../../../../../../drivers/cmsis/device/NXP/LPC17xx/include -I../../../../../../kernel/config -I../../../../../../kernel/core -I../../../../../../kernel/portable/device/NXP/LPC17xx -I../../../../../../middlewares/Log -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


