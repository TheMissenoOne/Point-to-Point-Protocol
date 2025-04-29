################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bsp.c \
../src/main.c \
../src/micro.c 

C_DEPS += \
./src/bsp.d \
./src/main.d \
./src/micro.d 

OBJS += \
./src/bsp.o \
./src/main.o \
./src/micro.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"C:\qp\qpc\include" -I"C:\Users\amaur\OneDrive\Documents\PSE\eclipse\Micro8\src" -I"C:\qp\qpc\ports\win32-qv" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/bsp.d ./src/bsp.o ./src/main.d ./src/main.o ./src/micro.d ./src/micro.o

.PHONY: clean-src

