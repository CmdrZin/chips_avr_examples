/* Arduino Emote Mod 8x8 Icon Demo */

#include <Wire.h>

/* *** LOCAL PROTOTYPES *** */
uint8_t makeHeader( uint8_t len );

#define  SLAVE_ADRS  0x60        // MUST match AVR chip I2C address

/* NOTE: ALL commands are a minimum of three bytes. LEN MOD CMD .. .. */

#define MOD_EM_SERVICE_ID  0x20

#define MES_SET_ICON       0x01
#define MES_SET_ICON_LEN   4

#define MES_LOAD_ICON       0x02
#define MES_LOAD_ICON_LEN   11

int slave = SLAVE_ADRS;   // has to be an int.
byte count = 0;           // simple counter
int cmdLen;

uint8_t outBuff[16];

uint8_t icon[8] = {
  0b00111100,
  0b01000010,
  0b10011001,
  0b10000001,
  0b10100101,
  0b10011001,
  0b01000010,
  0b00111100
};


void setup()
{
  Wire.begin();             // join i2c bus
}

void loop()
{
  uint8_t row;
  
  // Send SET CMD to Slave.
#if 1
  outBuff[0] = makeHeader( MES_SET_ICON_LEN-3 );
  outBuff[1] = MOD_EM_SERVICE_ID;
  outBuff[2] = MES_SET_ICON;
  if( count > 8 ) {
    count = 0;
  }
  outBuff[3] = count;
  ++count;
    
  cmdLen = MES_SET_ICON_LEN;
#endif

  // Send LOAD CMD to Slave.
#if 0
  outBuff[0] = makeHeader( MES_LOAD_ICON_LEN-3 );
  outBuff[1] = MOD_EM_SERVICE_ID;
  outBuff[2] = MES_LOAD_ICON;
  for( row = 0; row<8; ++row ) {
    outBuff[row+3] = icon[row];
  }
    
  cmdLen = MES_LOAD_ICON_LEN;
#endif

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit

  delay(1000);
}

uint8_t makeHeader( uint8_t len )
{
  uint8_t temp;
 
  temp = ((~len) << 4) & 0xF0;    // shift and mask
  temp = temp | (len & 0x0F);
  return (temp);
}

