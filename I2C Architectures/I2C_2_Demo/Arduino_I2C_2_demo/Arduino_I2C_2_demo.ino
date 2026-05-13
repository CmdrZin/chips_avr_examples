/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016-2026 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * 
 * Arduino I2C type 2 slave comm Demo
 * 12may26
 *
 */

#include <Wire.h>

#define SLAVE_ADRS  0x5E    // Motor Slave Base. MUST match AVR device I2C address.

// Write to Registers
#define STATUS_LED  0
#define LED_DELAY   1
#define CLR_CNT1    2
#define CLR_CNT2    3
#define CNT1_DIR    4
#define CNT2_DIR    5
#define SET_CNT1    6
// Read from Registers
#define ZERO_BUFF   0
#define STATUS_BUFF 1
#define CNT1_BUFF   2
#define CNT2_BUFF   3

#define STEP_TIME 200
#define MAX_STEPS 20
#define LED_WAIT  100
#define SYS_WAIT  1000

int slave = SLAVE_ADRS;      // has to be an int or a warning pops up.
int cmdLen;                  // number of bytes to send after the I2C SDA_W code.

uint16_t ledDelay = 500;

uint32_t ledTime = 0;
uint32_t sysUpdateTime = 0;

// Coomunications stuff
uint8_t outBuff[8];
uint8_t inBuff[8];
uint16_t count;
uint8_t index;
bool ledState = true;

uint32_t count1 = 0;

void setup()
{
  Wire.begin();               // enable i2c bus support
  Serial.begin(9600);        // enable serial. Set BAUD rate
  Serial.println("");
  Serial.print("Start example. Waiting for Slave at 0x");
  Serial.println(SLAVE_ADRS, HEX);
}

void loop()
{
  // Check I2C communications
#if 1
  // Turn OFF auto LED blink
  outBuff[0] = LED_DELAY;             // Register 1
  outBuff[1] = 0;                     // 0 disables auto LED
  cmdLen = 2;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(5);
#endif

#if 1
  // Use manual led control
  if( ledDelay == 0 ) {
    if( ledTime < millis() ) {
      ledTime = millis() + LED_WAIT;
      // Turn ON status LED
      outBuff[0] = 0;             // Register 0
      if(ledState) {
        outBuff[1] = 1;
        ledState = false;
      } else {
        outBuff[1] = 0;
        ledState = true;
      }
      cmdLen = 2;

      Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
      Wire.write(outBuff, cmdLen);        // send out data.
      Wire.endTransmission();             // complete transmission.

      delay(1);
    }
  }
#endif

#if 1
  if( sysUpdateTime < millis() ) {
    sysUpdateTime = millis() + SYS_WAIT;
    outBuff[0] = CNT1_BUFF;             // Read reg 2..count1
    cmdLen = 1;

    Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
    Wire.write(outBuff, cmdLen);        // send out data.
    Wire.endTransmission();             // complete transmission.

    delay(5);                        // wait for 5ms second.

    // Read the counter 1 (int32_t)
    // Issues a SLA_R request and triggers N reads before NACKing Slave.
    Wire.requestFrom(slave, 4);         // request 4 bytes from (int)slave
    // Read
    for(int i=0;i<4;i++) {
      while(!Wire.available());           // wait for DEV number.
      inBuff[i] = Wire.read();        // receive a byte as character
    }

    // rebuild counter 1
    count1 = inBuff[3];
    count1 = (count1<<8) + inBuff[2];
    count1 = (count1<<8) + inBuff[1];
    count1 = (count1<<8) + inBuff[0];

    Serial.print("Count1: ");
    Serial.println(count1);
  }
#endif
}
