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
../mod_led.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
main.o \
mod_led.o

OBJS_AS_ARGS +=  \
main.o \
mod_led.o

C_DEPS +=  \
main.d \
mod_led.d

C_DEPS_AS_ARGS +=  \
main.d \
mod_led.d

OUTPUT_FILE_PATH +=DemoApp1_ATtiny85.elf

OUTPUT_FILE_PATH_AS_ARGS +=DemoApp1_ATtiny85.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler





./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.9.2
	$(QUOTE)D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"D:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.0.79\include"  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=attiny85 -B "D:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.0.79\gcc\dev\attiny85" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
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
	@echo Invoking: AVR/GNU Linker : 4.9.2
	$(QUOTE)D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="DemoApp1_ATtiny85.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=attiny85 -B "D:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATtiny_DFP\1.0.79\gcc\dev\attiny85"  
	@echo Finished building target: $@
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "DemoApp1_ATtiny85.elf" "DemoApp1_ATtiny85.hex"
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "DemoApp1_ATtiny85.elf" "DemoApp1_ATtiny85.eep" || exit 0
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "DemoApp1_ATtiny85.elf" > "DemoApp1_ATtiny85.lss"
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "DemoApp1_ATtiny85.elf" "DemoApp1_ATtiny85.srec"
	"D:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "DemoApp1_ATtiny85.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "DemoApp1_ATtiny85.elf" "DemoApp1_ATtiny85.a" "DemoApp1_ATtiny85.hex" "DemoApp1_ATtiny85.lss" "DemoApp1_ATtiny85.eep" "DemoApp1_ATtiny85.map" "DemoApp1_ATtiny85.srec" "DemoApp1_ATtiny85.usersignatures"
	