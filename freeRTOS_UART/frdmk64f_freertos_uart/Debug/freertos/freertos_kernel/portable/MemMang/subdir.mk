################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../freertos/freertos_kernel/portable/MemMang/heap_4.c 

C_DEPS += \
./freertos/freertos_kernel/portable/MemMang/heap_4.d 

OBJS += \
./freertos/freertos_kernel/portable/MemMang/heap_4.o 


# Each subdirectory must supply rules for building sources it contributes
freertos/freertos_kernel/portable/MemMang/%.o: ../freertos/freertos_kernel/portable/MemMang/%.c freertos/freertos_kernel/portable/MemMang/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -DFRDM_K64F -DFREEDOM -DSERIAL_PORT_TYPE_UART=1 -DSDK_OS_FREE_RTOS -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\source" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\drivers" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\drivers\freertos" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\utilities" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\device" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\component\uart" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\component\serial_manager" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\component\lists" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\CMSIS" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\freertos\freertos_kernel\include" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\freertos\freertos_kernel\portable\GCC\ARM_CM4F" -I"C:\Users\HP\Documents\MCUXpressoIDE_11.6.0_8187\workspace\frdmk64f_freertos_uart\board" -O0 -fno-common -g3 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-freertos-2f-freertos_kernel-2f-portable-2f-MemMang

clean-freertos-2f-freertos_kernel-2f-portable-2f-MemMang:
	-$(RM) ./freertos/freertos_kernel/portable/MemMang/heap_4.d ./freertos/freertos_kernel/portable/MemMang/heap_4.o

.PHONY: clean-freertos-2f-freertos_kernel-2f-portable-2f-MemMang

