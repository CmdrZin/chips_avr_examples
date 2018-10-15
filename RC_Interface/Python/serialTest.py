#!/usr/bin/python

#
# The MIT License (MIT)
#
# Copyright (c) 2018 Nels D. "Chip" Pearson (aka CmdrZin)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# serialTest.py
#
# Created:  14oct18
# Author:   Chip
#

import serial

port = serial.Serial("/dev/ttyAMA0", baudrate=9600, timeout=3.0)

# turn ON LED
port.write(bytearray([0x5A,0x00,0x01]))

#default is Temperature at 1 Hz
print 'Temperature'
for i in range(10):
    rcv = port.readline()
    for i in rcv:
        print hex(ord(i)),
    print ' '

# Turn OFF LED    
port.write(bytearray([0x5A,0x00,0x00]))

# Switch to Throttle & Steering at 20ms (50 Hz)
port.write(bytearray([0x5A,0x11]))

print 'Throttle & Steering'
for i in range(10):
    rcv = port.readline()
    for i in rcv:
        print hex(ord(i)),
    print ' '

# Switch back to Temperature    
port.write(bytearray([0x5A,0x10]))
# get rid of extra data that was coming in at 20ms rate.
port.flushInput()

print 'Temperature'
for i in range(10):
    rcv = port.readline()
    for i in rcv:
        print hex(ord(i)),
    print ' '
