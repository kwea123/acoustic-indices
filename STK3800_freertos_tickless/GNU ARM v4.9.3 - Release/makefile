################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include FreeRTOS/efm32wg/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32WG/subdir.mk
-include BSP/subdir.mk
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
all: STK3800_freertos_tickless.axf

# Tool invocations
STK3800_freertos_tickless.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -T "STK3800_freertos_tickless.ld" -Xlinker --gc-sections -Xlinker -Map="STK3800_freertos_tickless.map" -mfpu=fpv4-sp-d16 -mfloat-abi=softfp --specs=nano.specs -o STK3800_freertos_tickless.axf "./BSP/bsp_trace.o" "./CMSIS/EFM32WG/startup_efm32wg.o" "./CMSIS/EFM32WG/system_efm32wg.o" "./Drivers/segmentlcd.o" "./Drivers/sleep.o" "./FreeRTOS/efm32wg/croutine.o" "./FreeRTOS/efm32wg/event_groups.o" "./FreeRTOS/efm32wg/heap_1.o" "./FreeRTOS/efm32wg/list.o" "./FreeRTOS/efm32wg/port.o" "./FreeRTOS/efm32wg/queue.o" "./FreeRTOS/efm32wg/tasks.o" "./FreeRTOS/efm32wg/timers.o" "./emlib/em_assert.o" "./emlib/em_burtc.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_lcd.o" "./emlib/em_rmu.o" "./emlib/em_rtc.o" "./emlib/em_system.o" "./src/low_power_tick_management.o" "./src/main.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: STK3800_freertos_tickless.hex'
	arm-none-eabi-objcopy -O ihex "STK3800_freertos_tickless.axf" "STK3800_freertos_tickless.hex"
	@echo ' '

	@echo 'Building bin file: STK3800_freertos_tickless.bin'
	arm-none-eabi-objcopy -O binary "STK3800_freertos_tickless.axf" "STK3800_freertos_tickless.bin"
	@echo ' '

	@echo 'Building s37 file: STK3800_freertos_tickless.s37'
	arm-none-eabi-objcopy -O srec "STK3800_freertos_tickless.axf" "STK3800_freertos_tickless.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "STK3800_freertos_tickless.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) STK3800_freertos_tickless.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
