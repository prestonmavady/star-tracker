################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/gv-data/catalog_bytestream.c \
../Core/gv-data/catalog_luts.c \
../Core/gv-data/catalog_xyz.c \
../Core/gv-data/tab_cat.c 

OBJS += \
./Core/gv-data/catalog_bytestream.o \
./Core/gv-data/catalog_luts.o \
./Core/gv-data/catalog_xyz.o \
./Core/gv-data/tab_cat.o 

C_DEPS += \
./Core/gv-data/catalog_bytestream.d \
./Core/gv-data/catalog_luts.d \
./Core/gv-data/catalog_xyz.d \
./Core/gv-data/tab_cat.d 


# Each subdirectory must supply rules for building sources it contributes
Core/gv-data/%.o Core/gv-data/%.su Core/gv-data/%.cyclo: ../Core/gv-data/%.c Core/gv-data/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4A6xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I"/Users/preston/Desktop/star-tracker/star-tracker/stm-32/Core/gv-data" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-gv-2d-data

clean-Core-2f-gv-2d-data:
	-$(RM) ./Core/gv-data/catalog_bytestream.cyclo ./Core/gv-data/catalog_bytestream.d ./Core/gv-data/catalog_bytestream.o ./Core/gv-data/catalog_bytestream.su ./Core/gv-data/catalog_luts.cyclo ./Core/gv-data/catalog_luts.d ./Core/gv-data/catalog_luts.o ./Core/gv-data/catalog_luts.su ./Core/gv-data/catalog_xyz.cyclo ./Core/gv-data/catalog_xyz.d ./Core/gv-data/catalog_xyz.o ./Core/gv-data/catalog_xyz.su ./Core/gv-data/tab_cat.cyclo ./Core/gv-data/tab_cat.d ./Core/gv-data/tab_cat.o ./Core/gv-data/tab_cat.su

.PHONY: clean-Core-2f-gv-2d-data

