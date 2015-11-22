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

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../access.c \
../core_functions.c \
../eeprom_util.c \
../function_tables.c \
../initialize.c \
../mod_adc.c \
../mod_dc_motor.c \
../pwm_dc_motor.c \
../mod_range_finder_IR.c \
../mod_range_finder_US.c \
../mod_led_status.c \
../service.c \
../sysTimer.c \
../TankBotBare2.c \
../twiSlave.c


PREPROCESSING_SRCS +=  \
../flash_table.s


ASM_SRCS += 


OBJS +=  \
access.o \
core_functions.o \
eeprom_util.o \
flash_table.o \
function_tables.o \
initialize.o \
mod_adc.o \
mod_dc_motor.o \
pwm_dc_motor.o \
mod_range_finder_IR.o \
mod_range_finder_US.o \
mod_led_status.o \
service.o \
sysTimer.o \
TankBotBare2.o \
twiSlave.o

OBJS_AS_ARGS +=  \
access.o \
core_functions.o \
eeprom_util.o \
flash_table.o \
function_tables.o \
initialize.o \
mod_adc.o \
mod_dc_motor.o \
pwm_dc_motor.o \
mod_range_finder_IR.o \
mod_range_finder_US.o \
mod_led_status.o \
service.o \
sysTimer.o \
TankBotBare2.o \
twiSlave.o

C_DEPS +=  \
access.d \
core_functions.d \
eeprom_util.d \
flash_table.d \
function_tables.d \
initialize.d \
mod_adc.d \
mod_dc_motor.d \
pwm_dc_motor.d \
mod_range_finder_IR.d \
mod_range_finder_US.d \
mod_led_status.d \
service.d \
sysTimer.d \
TankBotBare2.d \
twiSlave.d

C_DEPS_AS_ARGS +=  \
access.d \
core_functions.d \
eeprom_util.d \
flash_table.d \
function_tables.d \
initialize.d \
mod_adc.d \
mod_dc_motor.d \
pwm_dc_motor.d \
mod_range_finder_IR.d \
mod_range_finder_US.d \
mod_led_status.d \
service.d \
sysTimer.d \
TankBotBare2.d \
twiSlave.d

OUTPUT_FILE_PATH +=TankBotBare2.elf

OUTPUT_FILE_PATH_AS_ARGS +=TankBotBare2.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

# AVR32/GNU C Compiler































./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.8.1
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega164p -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler
./flash_table.o: .././flash_table.s
	@echo Building file: $<
	@echo Invoking: AVR32/GNU Assembler : 4.8.1
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=atmega164p -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<" 
	@echo Finished building: $<
	


./%.o: .././%.s
	@echo Building file: $<
	@echo Invoking: AVR32/GNU Assembler : 4.8.1
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=atmega164p -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<" 
	@echo Finished building: $<
	




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 4.8.1
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="TankBotBare2.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atmega164p  
	@echo Finished building target: $@
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "TankBotBare2.elf" "TankBotBare2.hex"
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "TankBotBare2.elf" "TankBotBare2.eep" || exit 0
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "TankBotBare2.elf" > "TankBotBare2.lss"
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "TankBotBare2.elf" "TankBotBare2.srec"
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-size.exe" "TankBotBare2.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "TankBotBare2.elf" "TankBotBare2.a" "TankBotBare2.hex" "TankBotBare2.lss" "TankBotBare2.eep" "TankBotBare2.map" "TankBotBare2.srec" "TankBotBare2.usersignatures"
	