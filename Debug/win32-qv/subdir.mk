################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/qp/qpc/ports/win32-qv/qf_port.c 

C_DEPS += \
./win32-qv/qf_port.d 

OBJS += \
./win32-qv/qf_port.o 


# Each subdirectory must supply rules for building sources it contributes
win32-qv/qf_port.o: C:/qp/qpc/ports/win32-qv/qf_port.c win32-qv/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"C:\qp\qpc\include" -I"C:\Users\amaur\OneDrive\Documents\PSE\eclipse\Micro8\src" -I"C:\qp\qpc\ports\win32-qv" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-win32-2d-qv

clean-win32-2d-qv:
	-$(RM) ./win32-qv/qf_port.d ./win32-qv/qf_port.o

.PHONY: clean-win32-2d-qv

