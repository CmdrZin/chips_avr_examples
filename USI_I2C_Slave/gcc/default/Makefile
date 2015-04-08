###############################################################################
# Makefile for the project USI_I2C_Slave_ATtiny84
###############################################################################

## General Flags
PROJECT = USI_I2C_Slave_ATtiny84
MCU = attiny84
TARGET = USI_I2C_Slave_ATtiny84.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=USI_I2C_Slave_ATtiny84.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"C:\Data\EmbeddedSystems\USI_I2C_Slave\gcc\src" 

## Objects that must be built in order to link
OBJECTS = main.o USI_TWI_Slave.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) USI_I2C_Slave_ATtiny84.hex USI_I2C_Slave_ATtiny84.eep USI_I2C_Slave_ATtiny84.lss## Compile
main.o: ../src/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

USI_TWI_Slave.o: ../src/USI_TWI_Slave.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) USI_I2C_Slave_ATtiny84.elf dep/* USI_I2C_Slave_ATtiny84.hex USI_I2C_Slave_ATtiny84.eep USI_I2C_Slave_ATtiny84.lss USI_I2C_Slave_ATtiny84.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

