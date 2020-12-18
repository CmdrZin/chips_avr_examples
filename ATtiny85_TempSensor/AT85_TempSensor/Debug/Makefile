################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../main.c \
../mod_adc.c \
../mod_led.c \
../usiTwiSlave.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
main.o \
mod_adc.o \
mod_led.o \
usiTwiSlave.o

OBJS_AS_ARGS +=  \
main.o \
mod_adc.o \
mod_led.o \
usiTwiSlave.o

C_DEPS +=  \
main.d \
mod_adc.d \
mod_led.d \
usiTwiSlave.d

C_DEPS_AS_ARGS +=  \
main.d \
mod_adc.d \
mod_led.d \
usiTwiSlave.d

OUTPUT_FILE_PATH +=AT85_TempSensor.elf

OUTPUT_FILE_PATH_AS_ARGS +=AT85_TempSensor.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler









./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"D:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.2.118\include"  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=attiny85 -B "D:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.2.118\gcc\dev\attiny85" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 5.4.0
	$(QUOTE)D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="AT85_TempSensor.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=attiny85 -B "D:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.2.118\gcc\dev\attiny85"  
	@echo Finished building target: $@
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "AT85_TempSensor.elf" "AT85_TempSensor.hex"
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "AT85_TempSensor.elf" "AT85_TempSensor.eep" || exit 0
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "AT85_TempSensor.elf" > "AT85_TempSensor.lss"
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "AT85_TempSensor.elf" "AT85_TempSensor.srec"
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "AT85_TempSensor.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "AT85_TempSensor.elf" "AT85_TempSensor.a" "AT85_TempSensor.hex" "AT85_TempSensor.lss" "AT85_TempSensor.eep" "AT85_TempSensor.map" "AT85_TempSensor.srec" "AT85_TempSensor.usersignatures"
	