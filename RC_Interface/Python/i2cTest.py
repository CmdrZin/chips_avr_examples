#!/usr/bin/python

# Multiple byte reads are not reliable on a Raspberry Pi.
# Even with a line change 1.36us before the clock edge.
# Use single byte reads.

# get the Python I2C driver interface.  Python 2.7.3 ONLY.
import smbus
import time

# version 2 uses port 1
bus = smbus.SMBus(1)

DEV_ADRS = 0x5C            # Sensor/LED Slave aboard address
DEV_STATUS_LED = 0x00      # Status LED command

LED_ON = 0x01
LED_OFF = 0x00

#out_val = [DEV_STATUS_LED, DSL_ON, 7]
out_block = [LED_OFF]

# Write out a BYTE to DEV_STATUS_LED register
bus.write_byte_data(DEV_ADRS, DEV_STATUS_LED, LED_ON)

# Does SDA_W REG then SDA_R DATA.
#print(hex(bus.read_byte_data(DEV_ADRS, 1)))
#print(hex(bus.read_byte_data(DEV_ADRS, 2)))
#print(hex(bus.read_byte_data(DEV_ADRS, 3)))
#print(hex(bus.read_byte_data(DEV_ADRS, 4)))
#print(hex(bus.read_byte_data(DEV_ADRS, 2)))
#print(hex(bus.read_byte_data(DEV_ADRS, 4)))
#print(hex(bus.read_byte_data(DEV_ADRS, 1)))

wt = time.clock() + 0.5
for c in range(5):
#    print(bus.read_i2c_block_data(DEV_ADRS, 1, 4))     # worked once or twice.
#    print(hex(bus.read_byte_data(DEV_ADRS, 2)))        # works once
#    print(hex(bus.read_byte(DEV_ADRS)))                # NAK after read, so can't read again.
#    print(bus.read_block_data(DEV_ADRS, 4))            # LOCKS UP SYSTEM

    # Multi stage command                               # works a couple of times
#    bus.write_byte(DEV_ADRS, 2)
#    bus.read_byte(DEV_ADRS)

# Issue seems RPi not responing to Slave clock stretch..trying 20MHz Slave.


    while( wt > time.clock() ):
        out_block[0] = 9
    wt += 0.5
    print(bus.read_i2c_block_data(DEV_ADRS, 1, 4))     # worked once or twice.
    print(c, 'Again\n')
## END of FOR loop
    
bus.write_byte_data(DEV_ADRS, DEV_STATUS_LED, LED_OFF)
