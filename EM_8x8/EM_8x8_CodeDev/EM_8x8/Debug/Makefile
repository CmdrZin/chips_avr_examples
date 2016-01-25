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
../EM_8x8.c \
../function_tables.c \
../i2c_address.c \
../initialize.c \
../led_display.c \
../mod_em_service.c \
../mod_led_status.c \
../service.c \
../sysTimer.c \
../twiSlave.c


PREPROCESSING_SRCS +=  \
../flash_table.s


ASM_SRCS += 


OBJS +=  \
access.o \
core_functions.o \
eeprom_util.o \
EM_8x8.o \
flash_table.o \
function_tables.o \
i2c_address.o \
initialize.o \
led_display.o \
mod_em_service.o \
mod_led_status.o \
service.o \
sysTimer.o \
twiSlave.o

OBJS_AS_ARGS +=  \
access.o \
core_functions.o \
eeprom_util.o \
EM_8x8.o \
flash_table.o \
function_tables.o \
i2c_address.o \
initialize.o \
led_display.o \
mod_em_service.o \
mod_led_status.o \
service.o \
sysTimer.o \
twiSlave.o

C_DEPS +=  \
access.d \
core_functions.d \
eeprom_util.d \
EM_8x8.d \
flash_table.d \
function_tables.d \
i2c_address.d \
initialize.d \
led_display.d \
mod_em_service.d \
mod_led_status.d \
service.d \
sysTimer.d \
twiSlave.d

C_DEPS_AS_ARGS +=  \
access.d \
core_functions.d \
eeprom_util.d \
EM_8x8.d \
flash_table.d \
function_tables.d \
i2c_address.d \
initialize.d \
led_display.d \
mod_em_service.d \
mod_led_status.d \
service.d \
sysTimer.d \
twiSlave.d

OUTPUT_FILE_PATH +=EM_8x8.elf

OUTPUT_FILE_PATH_AS_ARGS +=EM_8x8.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

# AVR32/GNU C Compiler



























./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.8.1
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega328p -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler
./flash_table.o: .././flash_table.s
	@echo Building file: $<
	@echo Invoking: AVR32/GNU Assembler : 4.8.1
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=atmega328p -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<" 
	@echo Finished building: $<
	


./%.o: .././%.s
	@echo Building file: $<
	@echo Invoking: AVR32/GNU Assembler : 4.8.1
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=atmega328p -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<" 
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
	$(QUOTE)D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="EM_8x8.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atmega328p  
	@echo Finished building target: $@
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "EM_8x8.elf" "EM_8x8.hex"
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "EM_8x8.elf" "EM_8x8.eep" || exit 0
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "EM_8x8.elf" > "EM_8x8.lss"
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "EM_8x8.elf" "EM_8x8.srec"
	"D:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1056\avr8-gnu-toolchain\bin\avr-size.exe" "EM_8x8.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "EM_8x8.elf" "EM_8x8.a" "EM_8x8.hex" "EM_8x8.lss" "EM_8x8.eep" "EM_8x8.map" "EM_8x8.srec" "EM_8x8.usersignatures"
	