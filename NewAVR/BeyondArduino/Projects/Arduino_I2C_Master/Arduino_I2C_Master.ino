/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016-2018 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * Arduino as I2C Master Demo for twiRegSlave project
 * 12nov20
 *
 * Every 200 ms
 * This demo writes to register 1 (LED control) alternating 1 and 0.
 * It reads registers 1&2 which have either a count or the device temperature.
 */

#include <Wire.h>

#define SLAVE_ADRS  0x50    // MUST match AVR chip I2C address
// READ registers
#define R_TEMP_H_REG    0x01
#define R_TEMP_L_REG    0x02
#define R_ADC_REG   0x01
// WRITE resigters
#define W_LED_REG       0x01
#define LED_OFF         0x00
#define LED_ON          0x01

int slave = SLAVE_ADRS;      // has to be an int or a warning pops up.
int cmdLen;                  // number of bytes to send after the I2C SDA_W code.

uint8_t outBuff[4];
uint8_t inBuff[4];
uint8_t count;

void setup()
{
  Wire.begin();               // enable i2c bus support
  Wire.setWireTimeout(30000, true);
  Serial.begin(9600);        // enable serial. Set BAUD rate
  Serial.print("\nStart example. Waiting for Slave at 0x");
  Serial.println(SLAVE_ADRS, HEX);
}

void loop()
{
  char numBytes = 0;

  // Do App
#if 0
  // Send LED ON Command to Slave.
  outBuff[0] = W_LED_REG;       // 0x00 is LED cmd register.
  outBuff[1] = LED_ON;          // A 0x00 turns OFF the LED. A 0x01 turns it ON.
  cmdLen = 2;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(200);                        // wait for 1 second.

  // Send LED OFF Command to Slave.
  outBuff[0] = W_LED_REG;       // 0x00 is LED cmd register.
  outBuff[1] = LED_OFF;              // A 0x00 turns OFF the LED. A 0x01 turns it ON.
  cmdLen = 2;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(200);                        // wait for 1 second.
#endif

#if 1
  // Send read Temperature message to Slave.
  outBuff[0] = R_ADC_REG;         // Set register index to 1.
  cmdLen = 1;

  Serial.println("Sending Read Slave.");

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  numBytes = Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  if(numBytes != 1) {
    Serial.println("Read ADRS Failed.");
  } else {
    Serial.println("Read Slave sent.");
  }

  delay(1);
  
  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, 1);         // request 1 byte from (int)slave
  // Read
  unsigned char d;
  while(Wire.available()) {
    d = Wire.read();        // receive a byte as character
 
    Serial.print("ADC DEC:");
    Serial.print(d, DEC);
    Serial.print(" HEX:");
    Serial.print(d,HEX);
    Serial.println(" ");
  }

  delay(1000);
#endif

#if 0
// Read four bytes
  outBuff[0] = R_TEMP_H_REG;                // Set register index to 1.
  cmdLen = 1;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(1);
  
  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, 4);         // request 2 bytes from (int)slave
  // Read
  while(!Wire.available());
  unsigned char dr1 = Wire.read();        // receive a byte as character

  while(!Wire.available());
  unsigned char dr2 = Wire.read();        // receive a byte as character

  while(!Wire.available());
  unsigned char dr3 = Wire.read();        // receive a byte as character

  while(!Wire.available());
  unsigned char dr4 = Wire.read();        // receive a byte as character

  Serial.print("R1 - R4: ");
  Serial.print(dr1, HEX);
  Serial.print(" ");
  Serial.print(dr2, HEX);
  Serial.print(" ");
  Serial.print(dr3, HEX);
  Serial.print(" ");
  Serial.print(dr4, HEX);
  Serial.println(" ");

#endif

  delay(10);                        // wait for 1 second.

  ++count;
  Serial.print("Loops: ");
  Serial.println(count);
}
