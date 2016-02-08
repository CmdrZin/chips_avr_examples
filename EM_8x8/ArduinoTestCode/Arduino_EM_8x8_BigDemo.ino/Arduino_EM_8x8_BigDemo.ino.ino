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
 * Arduino Emote Mod 8x8 Big Demo
 * This demo shows all of the modes and some possibilities of the EM 8x8.
 */

#include <Wire.h>

/* *** LOCAL PROTOTYPES *** */
void sendMarqueeList( uint8_t* buf, uint8_t size);
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

typedef struct ANIM_DATA {
  uint8_t icon;
  uint16_t displayTime;
} ANIM_ELEMENT;

int slave = SLAVE_ADRS;   // has to be an int.
byte count = 0;           // simple counter
uint8_t demoState;
bool scanState = true;    // toggle state to change direction of scan
unsigned long waitTime;
unsigned long lastTime;
uint8_t animState;

uint8_t outBuff[20];

// ICON buffer
uint8_t iconBuf[8] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};


// Animation lists
ANIM_ELEMENT animListL[] = {
  { 0,  500},      // normal eye..wait here for .5 seconds
  {20,  200},
  {13,  200},
  {14,  200},
  {15,  200},
  {16,  200},
  {17,  200},
  {18,  200},
  {19,  200},
  {20,  200}
};

ANIM_ELEMENT animListW[8] = {
  { 0,  2500},      // normal eye..wait here for 2.5 seconds
  {12,  20},     // close1
  {11,  20},
  {10,  20},
  { 9,  20},      // closed
  {10,  20},
  {11,  20},
  {12,  20}
};

// Marquee lists
uint8_t icons09[] = { 32, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 32 };
uint8_t iconsAM[] = { 32, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77 };
uint8_t iconsNZ[] = { 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 32 };

void setup()
{
  Wire.begin();             // join i2c bus
  demoState = 5;
  count = 0;
  lastTime = 0;
  animState = 0;
  waitTime = 1000;
}

void loop()
{
  int     cmdLen;
  int     row;
  unsigned long time;
  uint8_t icon;

  switch( demoState )
  {
    case 0:     // Random
      // Update icon
      for( row=0; row<8; ++row )
      {
        iconBuf[row] = (byte)random(++count);
      }
      // Send CMD to Slave.
      outBuff[0] = makeHeader( MES_LOAD_ICON_LEN-3 );
      outBuff[1] = MOD_EM_SERVICE_ID;
      outBuff[2] = MES_LOAD_ICON;
      for( row = 0; row<8; ++row ) {
        outBuff[row+3] = iconBuf[row];
      }
      cmdLen = MES_LOAD_ICON_LEN;
      Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
      Wire.write(outBuff, cmdLen);       // sends device ID and data
      Wire.endTransmission();            // complete transmit
      delay(200);

      if(++count > 450)
      {
        count = 0;
        demoState = 1;
        for(row=0; row<8; ++row)
          iconBuf[row] = 0;
      }
      break;

    case 1:     // Scan 1
      // Update icon
      if( scanState )
      {
        iconBuf[3] <<= 1;
        if( iconBuf[3] == 0 )
        {
          iconBuf[3] = 0x80;
          scanState = !scanState;
        }
      }
      else
      {
        iconBuf[3] >>= 1;
        if( iconBuf[3] == 0 )
        {
          iconBuf[3] = 0x01;
          scanState = !scanState;
        }
      }
      // Send CMD to Slave.
      outBuff[0] = makeHeader( MES_LOAD_ICON_LEN-3 );
      outBuff[1] = MOD_EM_SERVICE_ID;
      outBuff[2] = MES_LOAD_ICON;
      for( row = 0; row<8; ++row ) {
        outBuff[row+3] = iconBuf[row];
      }
      cmdLen = MES_LOAD_ICON_LEN;
      Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
      Wire.write(outBuff, cmdLen);       // sends device ID and data
      Wire.endTransmission();            // complete transmit
      delay(200);

      if(++count > 50)
      {
        count = 0;
        demoState = 2;
      }
      break;

    case 2:       // Scan 8
      // Update icon
      if( scanState )
      {
        for( row=0; row<8; ++row )
          iconBuf[row] <<= 1;
        if( iconBuf[3] == 0 )
        {
          for( row=0; row<8; ++row )
            iconBuf[row] = 0x80;
          scanState = !scanState;
        }
      }
      else
      {
        for( row=0; row<8; ++row )
          iconBuf[row] >>= 1;
        if( iconBuf[3] == 0 )
        {
          for( row=0; row<8; ++row )
            iconBuf[row] = 0x01;
          scanState = !scanState;
        }
      }
      // Send CMD to Slave.
      outBuff[0] = makeHeader( MES_LOAD_ICON_LEN-3 );
      outBuff[1] = MOD_EM_SERVICE_ID;
      outBuff[2] = MES_LOAD_ICON;
      for( row = 0; row<8; ++row ) {
        outBuff[row+3] = iconBuf[row];
      }
      cmdLen = MES_LOAD_ICON_LEN;     
      Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
      Wire.write(outBuff, cmdLen);       // sends device ID and data
      Wire.endTransmission();            // complete transmit
      delay(200);
      if(++count > 50)
      {
        count = 0;
        demoState = 3;
      }
      break;

    case 3:       // Animation Loop
      // check timer
      time = millis();
  
      if( (time - lastTime) > waitTime )
      {
        lastTime = time;

        // Get Icon to display
        icon = animListL[ animState ].icon;
        // Get duration to display it.
        waitTime = (unsigned long)animListL[ animState ].displayTime;
  
        if( ++animState >= (sizeof(animListL)/sizeof(ANIM_ELEMENT)) )
        {
          animState = 0;
        }
        // Send SET CMD to Slave.
        outBuff[0] = makeHeader( MES_SET_ICON_LEN-3 );
        outBuff[1] = MOD_EM_SERVICE_ID;
        outBuff[2] = MES_SET_ICON;
        outBuff[3] = icon;
        cmdLen = MES_SET_ICON_LEN;
        Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
        Wire.write(outBuff, cmdLen);       // sends device ID and data
        Wire.endTransmission();            // complete transmit
        if(++count > 50)
        {
          count = 0;
          demoState = 4;
          animState = 0;
        }
      }
      break;
    
    case 4:       // Animation Wink
      // check timer
      time = millis();
  
      if( (time - lastTime) > waitTime )
      {
        lastTime = time;

        // Get Icon to display
        icon = animListW[ animState ].icon;
        // Get duration to display it.
        waitTime = (unsigned long)animListW[ animState ].displayTime;
  
        if( ++animState >= (sizeof(animListW)/sizeof(ANIM_ELEMENT)) )
        {
          animState = 0;
        }
        // Send SET CMD to Slave.
        outBuff[0] = makeHeader( MES_SET_ICON_LEN-3 );
        outBuff[1] = MOD_EM_SERVICE_ID;
        outBuff[2] = MES_SET_ICON;
        outBuff[3] = icon;
        cmdLen = MES_SET_ICON_LEN;
        Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
        Wire.write(outBuff, cmdLen);       // sends device ID and data
        Wire.endTransmission();            // complete transmit
        if(++count > 50)
        {
          count = 0;
          demoState = 5;
        }
      }
      break;

    case 5:
      sendMarqueeList( icons09, sizeof(icons09) );
      delay(8000);
      sendMarqueeList( iconsAM, sizeof(iconsAM) );
      delay(10055);
      sendMarqueeList( iconsNZ, sizeof(iconsNZ) );
      delay(12000);
//      demoState = 0;
      break;
  }

}

/*
 * Send Marquee list
 */
void sendMarqueeList( uint8_t* buf, uint8_t size)
{
  uint8_t index;
  int     cmdLen;

  cmdLen = MES_LOAD_MSERIES_LEN+size;
  // Load Marquee icon series.
  outBuff[0] = makeHeader( cmdLen-3 );
  outBuff[1] = MOD_EM_SERVICE_ID;
  outBuff[2] = MES_LOAD_MSERIES;
  for( index=0; index<size; ++index ) {
    outBuff[index+3] = buf[index];
  }
  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit

  // Enable Marquee
  outBuff[0] = makeHeader( MES_SEL_MARQUEE_LEN-3 );
  outBuff[1] = MOD_EM_SERVICE_ID;
  outBuff[2] = MES_SEL_MARQUEE;
  cmdLen = MES_SEL_MARQUEE_LEN;
  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
}

/*
 * This is a simple utility to build the first byte of the
 * header by generating the one comp of the lower 4 bits.
 */
uint8_t makeHeader( uint8_t len )
{
  uint8_t temp;
 
  temp = ((~len) << 4) & 0xF0;    // shift and mask
  temp = temp | (len & 0x0F);
  return (temp);
}

