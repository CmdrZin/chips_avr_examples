/* Arduino Emote Mod 8x8 Load Icon Test - Scanner */
/* Update the icon buffer each cycle to animate. */

#include <Wire.h>

#define SERIAL_USB  1

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
bool state = true;        // toggle state to change direction of scan

uint8_t outBuff[16];

uint8_t icon[8] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};


void setup()
{
  Wire.begin();             // join i2c bus
#if(SERIAL_USB == 1)
  Serial.begin(57600);      // Set BAUD rate
#endif
}

void loop()
{
  uint8_t row;
  
#if(SERIAL_USB == 1)
  Serial.print("Count ");
  Serial.println(count++, DEC);
#else
  ++count;
#endif

  // Update icon
  if( state )
  {
    icon[3] <<= 1;
    if( icon[3] == 0 )
    {
      icon[3] = 0x80;
      state = !state;
    }
  }
  else
  {
    icon[3] >>= 1;
    if( icon[3] == 0 )
    {
      icon[3] = 0x01;
      state = !state;
    }
  }
  
  // Send CMD to Slave.
  outBuff[0] = makeHeader( MES_LOAD_ICON_LEN-3 );
  outBuff[1] = MOD_EM_SERVICE_ID;
  outBuff[2] = MES_LOAD_ICON;
  for( row = 0; row<8; ++row ) {
    outBuff[row+3] = icon[row];
  }
    
  cmdLen = MES_LOAD_ICON_LEN;

#if(SERIAL_USB == 1)
  Serial.print(outBuff[0], HEX);
  Serial.print(" ");
  Serial.print(outBuff[1], HEX);
  Serial.print(" ");
  Serial.print(outBuff[2], HEX);
  Serial.print(" ");
  Serial.print(outBuff[3], HEX);
  Serial.print(" ");
  Serial.print(outBuff[4], HEX);
  Serial.print(" ");
  Serial.print(outBuff[5], HEX);
  Serial.print(" ");
  Serial.print(outBuff[6], HEX);
  Serial.print(" ");
  Serial.print(outBuff[7], HEX);
  Serial.print(" ");
  Serial.print(outBuff[8], HEX);
  Serial.print(" ");
  Serial.print(outBuff[9], HEX);
  Serial.print(" ");
  Serial.println(outBuff[10], HEX);
#endif

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit

  delay(200);
}

uint8_t makeHeader( uint8_t len )
{
  uint8_t temp;
 
  temp = ((~len) << 4) & 0xF0;    // shift and mask
  temp = temp | (len & 0x0F);
  return (temp);
}

