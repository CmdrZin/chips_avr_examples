/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
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
 */

/*
 * Arduino Emote Mod 8x8 Load Marquee 01 Test
 * This tests the Marquee function be loading a list of icons for numbers and
 * then enabling the Marquee function.
 */

#include <Wire.h>

/* *** LOCAL PROTOTYPES *** */
uint8_t makeHeader( uint8_t len );

#define  SLAVE_ADRS  0x60        // MUST match AVR chip I2C address

/* NOTE: ALL commands are a minimum of three bytes. LEN MOD CMD .. .. */

#define MOD_EM_SERVICE_ID  0x20

#define MES_SET_ICON          0x01
#define MES_SET_ICON_LEN      4

#define MES_LOAD_ICON         0x02
#define MES_LOAD_ICON_LEN     11

#define MES_SEL_MARQUEE       0x40
#define MES_SEL_MARQUEE_LEN   3

#define MES_SET_KERNING       0x41
#define MES_SET_KERNING_LEN   4

#define MES_SET_CONT_FLAG     0x42
#define MES_SET_CONT_FLAG_LEN 4

#define MES_SET_RATE          0x43
#define MES_SET_RATE_LEN      4

#define MES_LOAD_MSERIES      0x44
#define MES_LOAD_MSERIES_LEN  3


#define  CORE_ID  254
#define  GET_BUILD_TIME  0x0A
#define  GET_BUILD_TIME_LEN  3
#define  GET_BUILD_TIME_DATA_LEN  9

#define  GET_BUILD_DATE  0x0B
#define  GET_BUILD_DATE_DATA_LEN  12
#define  GET_BUILD_DATE_LEN  3

#define  GET_DB_VERSION  0x02
#define  GET_DB_VERSION_LEN  3
#define  GET_DB_VERSION_DATA_LEN  5

int slave = SLAVE_ADRS;   // has to be an int.
byte count = 0;           // simple counter
int cmdLen;

uint8_t outBuff[16];

uint8_t icons[] = { 49, 50, 51, 52, 53, 54, 55, 56, 57, 32 };


void setup()
{
  Wire.begin();             // join i2c bus
}

void loop()
{
  uint8_t index;
  
  cmdLen = MES_LOAD_MSERIES_LEN+sizeof(icons);
  // Load Marquee icon series.
  outBuff[0] = makeHeader( cmdLen-3 );
  outBuff[1] = MOD_EM_SERVICE_ID;
  outBuff[2] = MES_LOAD_MSERIES;
  for( index=0; index<sizeof(icons); ++index ) {
    outBuff[index+3] = icons[index];
  }
  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.

  // Enable Marquee
  outBuff[0] = makeHeader( MES_SEL_MARQUEE_LEN-3 );
  outBuff[1] = MOD_EM_SERVICE_ID;
  outBuff[2] = MES_SEL_MARQUEE;
  cmdLen = MES_SEL_MARQUEE_LEN;
  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.


//#define MES_SET_KERNING       0x41
//#define MES_SET_KERNING_LEN   4

//#define MES_SET_CONT_FLAG     0x42
//#define MES_SET_CONT_FLAG_LEN 4

//#define MES_SET_RATE          0x43
//#define MES_SET_RATE_LEN      4

  delay(10000);
}

uint8_t makeHeader( uint8_t len )
{
  uint8_t temp;
 
  temp = ((~len) << 4) & 0xF0;    // shift and mask
  temp = temp | (len & 0x0F);
  return (temp);
}

