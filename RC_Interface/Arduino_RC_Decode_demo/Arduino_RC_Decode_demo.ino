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
 * Arduino AT85 RC Slave Demo
 * 26aug18
 *
 * This demo code will cause the LED to blink ON and OFF at a 1 second rate.
 * And read back ADC Ch4 (Temperature) and Throttle and Steering servo inputs.
 * NOTE: Serial Monitor MUST be active for code to run.
 */

#include <Wire.h>

#define SLAVE_ADRS  0x5C    // MUST match AVR chip I2C address

#define STATUS_LED_REG  0x00
#define LED_OFF           0x00
#define LED_ON            0x01
#define TEMP_H          0x01
#define TEMP_L          0x02
#define THROTTLE        0x03
#define STEERING        0x04

int slave = SLAVE_ADRS;      // has to be an int or a warning pops up.
int cmdLen;                  // number of bytes to send after the I2C SDA_W code.

uint8_t outBuff[4];
uint8_t inBuff[4];
uint8_t count;

void setup()
{
  Wire.begin();               // enable i2c bus support
  Serial.begin(9600);        // enable serial. Set BAUD rate
  Serial.print("Start example. Waiting for Slave at 0x");
  Serial.println(SLAVE_ADRS, HEX);
}

void loop()
{
  // Do App
  // Send LED ON Command to Slave.
  outBuff[0] = STATUS_LED_REG;       // 0x00 is LED cmd register.
  outBuff[1] = LED_OFF;              // A 0x00 turns OFF the LED. A 0x01 turns it ON.
  cmdLen = 2;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(100);                        // wait for 1 second.

  // Send LED OFF Command to Slave.
  outBuff[0] = STATUS_LED_REG;       // 0x00 is LED cmd register.
  outBuff[1] = LED_ON;               // A 0x00 turns OFF the LED. A 0x01 turns it ON.
  cmdLen = 2;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(10);                        // wait for 1 second.
#if 1
  // Send read Temperature message to Slave.
  outBuff[0] = TEMP_H;                // Set register index to 1.
  cmdLen = 1;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(1);
  
  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, 2);         // request 2 bytes from (int)slave
  // Read
  while(!Wire.available());
  unsigned char dh = Wire.read();        // receive a byte as character

  while(!Wire.available());
  unsigned char dl = Wire.read();        // receive a byte as character

  unsigned int d = (dh<<8)+dl;
  Serial.print("Temperature: ");
  Serial.print(d, DEC);

  Serial.print(" ");
  Serial.print(dh,HEX);
  Serial.print(dl,HEX);

  Serial.println(" ");

  // Send read THROTTLE data from Slave.
  outBuff[0] = THROTTLE;                // Set register index.
  cmdLen = 1;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(1);

  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, 1);         // request 1 byte from (int)slave
  // Read
  while(!Wire.available());
  unsigned char thr = Wire.read();        // receive a byte as character

  Serial.print("Throttle: ");
  Serial.print(thr, HEX);
  Serial.println(" ");

  // Send read STEERING data from Slave.
  outBuff[0] = STEERING;                // Set register index.
  cmdLen = 1;

  Wire.beginTransmission(slave);      // identify the Slave to transmit to. Have to do this each time.
  Wire.write(outBuff, cmdLen);        // send out data.
  Wire.endTransmission();             // complete transmission.

  delay(1);

  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, 1);         // request 1 byte from (int)slave
  // Read
  while(!Wire.available());           // wait for DEV number.
  unsigned char ste = Wire.read();        // receive a byte as character

  Serial.print("Steering: ");
  Serial.print(ste, HEX);
  Serial.println(" ");
#endif

#if 1
// Read four bytes
  outBuff[0] = TEMP_H;                // Set register index to 1.
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

  delay(1000);                        // wait for 1 second.

  ++count;
  Serial.print("Loops: ");
  Serial.println(count);
}

