################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/Log/subdir.mk
-include Kernel/Port/LPC17xx/subdir.mk
-include Kernel/Core/subdir.mk
-include Drivers/LPC17xx_HAL_Drivers/subdir.mk
-include Drivers/CMSIS/LPC17xx/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/EA-XPR-021/subdir.mk
-include Application/MultiEvents/subdir.mk
-include Application/LPCXPRESSO/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: EventOS_example.axf

# Tool invocations
EventOS_example.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -Xlinker -Map="EventOS_example.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "EventOS_example_Debug.ld" -o "EventOS_example.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) EventOS_example.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "EventOS_example.axf"; # arm-none-eabi-objcopy -v -O binary "EventOS_example.axf" "EventOS_example.bin" ; # checksum -p LPC1769 -d "EventOS_example.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
