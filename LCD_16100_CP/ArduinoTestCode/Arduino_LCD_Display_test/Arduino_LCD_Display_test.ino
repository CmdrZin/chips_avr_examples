/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016-2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * Arduino LCD Display I2C Slave Demo
 * 21dec17
 *
 * Ref: DN_LCD_16100_CP.rtf - API
 * 
 * NOTE: Activate Serial Monitor
 */

#include <Wire.h>

#define SERIAL_USB  0

/* *** LOCAL PROTOTYPES *** */

#define  SLAVE_ADRS  0x5E        // MUST match AVR chip I2C address


#define MLCD_TEXT    1

#define IO_GET_BUTTONS           0x01
#define IO_GET_BUTTONS_LEN       1
#define IO_GET_BUTTONS_DATA_LEN  1

#define IO_GREEN_LED    0x20
#define IO_RED_LED      0x21
#define IO_YELLOW_LED   0x22
#define IO_LED_LEN      2

#define  GET_BUILD_TIME  0xF8
#define  GET_BUILD_TIME_LEN  3
#define  GET_BUILD_TIME_DATA_LEN  9

#define  GET_BUILD_DATE  0xF9
#define  GET_BUILD_DATE_DATA_LEN  12
#define  GET_BUILD_DATE_LEN  3

#define  GET_DB_VERSION  0xFA
#define  GET_DB_VERSION_LEN  3
#define  GET_DB_VERSION_DATA_LEN  5

int slave = SLAVE_ADRS;   // has to be an int.
byte count = 0;           // simple counter
int cmdLen;
uint8_t outBuff[16];

void setup()
{
  Wire.begin();             // join i2c bus
#if(SERIAL_USB == 1)
  Serial.begin(57600);      // Set BAUD rate
#endif
}

void loop()
{
  int data_len;
  
#if(SERIAL_USB == 1)
  Serial.print("Count ");
  Serial.println(count++, DEC);
#else
  ++count;
#endif
  
// Send CMD to Slave.
#if 1
#if 1
  if( count % 3 == 2 ) {
  outBuff[0] = IO_GET_BUTTONS;
  cmdLen = IO_GET_BUTTONS_LEN;
  data_len = IO_GET_BUTTONS_DATA_LEN;
  }
#endif
  Serial.print(slave, HEX);
  Serial.print(": ");
  Serial.println(outBuff[0], HEX);

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.

#if 1
  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, data_len);    // request N bytes from (int)slave

  // Flush buffer
  while(!Wire.available());    // wait for DEV number.
  unsigned char d = Wire.read();        // receive a byte as character
  Serial.print(d, HEX);
  Serial.print(", ");

  while(Wire.available())        // slave may send less than requested
  { 
    unsigned char d = Wire.read();        // receive a byte as character
    Serial.print(d, HEX);
  }
  Serial.println(".");
#endif
#endif

#if 1
  switch(count % 6) {
    case 0:
      outBuff[0] = IO_GREEN_LED;
      outBuff[1] = 1;
      break;
    case 1:
      outBuff[0] = IO_GREEN_LED;
      outBuff[1] = 0;
      break;
    case 2:
      outBuff[0] = IO_RED_LED;
      outBuff[1] = 1;
      break;
    case 3:        
      outBuff[0] = IO_RED_LED;
      outBuff[1] = 0;
      break;
    case 4:
      outBuff[0] = IO_YELLOW_LED;
      outBuff[1] = 1;
      break;
    case 5:
      outBuff[0] = IO_YELLOW_LED;
      outBuff[1] = 0;
      break;
  }
  
  cmdLen = IO_LED_LEN;

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.
#endif

#if 0
  outBuff[0] = makeHeader( 8-3 );
  outBuff[1] = MOD_LCD_ID;
  outBuff[2] = MLCD_TEXT;
  outBuff[3] = 2;
  outBuff[4] = 'H';
  outBuff[5] = 'e';
  outBuff[6] = ' ';
  outBuff[7] = '7';
    
  cmdLen = 8;

#if(SERIAL_USB == 1)
  Serial.print(outBuff[0], HEX);
  Serial.print(" ");
  Serial.print(outBuff[1], HEX);
  Serial.print(" ");
  Serial.print(outBuff[2], HEX);
  Serial.print(" ");
  Serial.println(outBuff[3]);
#endif

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.
#endif


#if 0
#if 1
  if( count % 3 == 0 ) {
  outBuff[0] = makeHeader( GET_BUILD_TIME_LEN-3 );
  outBuff[1] = CORE_ID;
  outBuff[2] = GET_BUILD_TIME;
  cmdLen = GET_BUILD_TIME_LEN;
  data_len = GET_BUILD_TIME_DATA_LEN;
  }
#endif
#if 1
  if( count % 3 == 1 ) {
  outBuff[0] = makeHeader( GET_BUILD_DATE_LEN-3 );
  outBuff[1] = CORE_ID;
  outBuff[2] = GET_BUILD_DATE;
  cmdLen = GET_BUILD_DATE_LEN;
  data_len = GET_BUILD_DATE_DATA_LEN;
  }
#endif
#if 1
  if( count % 3 == 2 ) {
  outBuff[0] = makeHeader( GET_DB_VERSION_LEN-3 );
  outBuff[1] = CORE_ID;
  outBuff[2] = GET_DB_VERSION;
  cmdLen = GET_DB_VERSION_LEN;
  data_len = GET_DB_VERSION_DATA_LEN;
  }
#endif
  Serial.print(outBuff[0], HEX);
  Serial.print(" ");
  Serial.print(outBuff[1], HEX);
  Serial.print(" ");
  Serial.println(outBuff[2], HEX);

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.

#if 1
  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, data_len);    // request N bytes from (int)slave

  // Flush buffer
  while(!Wire.available());    // wait for DEV number.
  unsigned char d = Wire.read();        // receive a byte as character
  Serial.print(d, HEX);
  Serial.print(" ");

  while(Wire.available())        // slave may send less than requested
  { 
    char c = Wire.read();        // receive a byte as character
    Serial.print(c);
  }
  Serial.println("");
#endif

#endif

  delay(1000);
}

