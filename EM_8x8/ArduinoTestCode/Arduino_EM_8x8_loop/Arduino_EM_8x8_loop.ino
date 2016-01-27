/* Arduino Emote Mod 8x8 Loop Animation Demo */

#include <Wire.h>

#define SERIAL_USB  0            // TEST

/* *** LOCAL PROTOTYPES *** */
uint8_t makeHeader( uint8_t len );

#define  SLAVE_ADRS  0x60        // MUST match AVR chip I2C address

/* NOTE: ALL commands are a minimum of three bytes. LEN MOD CMD .. .. */

#define MOD_EM_SERVICE_ID  0x20

#define MES_SET_ICON       0x01
#define MES_SET_ICON_LEN   4

#define MES_LOAD_ICON       0x02
#define MES_LOAD_ICON_LEN   11

typedef struct ANIM_DATA {
  uint8_t icon;
  uint16_t displayTime;
} ANIM_ELEMENT;


int slave = SLAVE_ADRS;   // has to be an int.
byte count = 0;           // simple counter
int cmdLen;
unsigned long waitTime;
unsigned long lastTime;
uint8_t animState;

uint8_t outBuff[16];

// List of icons to cycle through
ANIM_ELEMENT animList[] = {
  { 0, 5000},      // normal eye..wait here for 5 seconds
  {20,  200},     // close1
  {13,  200},
  {14,  200},
  {15,  200},      // closed
  {16,  200},
  {17,  200},
  {18,  200},
  {19,  200},
  {20,  200}
};


void setup()
{
  Wire.begin();             // join i2c bus

#if(SERIAL_USB == 1)
  Serial.begin(57600);      // Set BAUD rate
#endif

  lastTime = 0;
  animState = 0;
  waitTime = 1000;
}

void loop()
{
  unsigned long time;
  uint8_t icon;
  
  // check timer
  time = millis();
  
  if( (time - lastTime) > waitTime )
  {
    lastTime = time;

    // Get Icon to display
    icon = animList[ animState ].icon;
    // Get duration to display it.
    waitTime = (unsigned long)animList[ animState ].displayTime;
  
    if( ++animState >= (sizeof(animList)/sizeof(ANIM_ELEMENT)) )
    {
      animState = 0;
    }

#if(SERIAL_USB == 1)
  Serial.print(icon, DEC);
  Serial.print(" ");
  Serial.println(waitTime, HEX);
#endif
    
    // Send SET CMD to Slave.
    outBuff[0] = makeHeader( MES_SET_ICON_LEN-3 );
    outBuff[1] = MOD_EM_SERVICE_ID;
    outBuff[2] = MES_SET_ICON;
    outBuff[3] = icon;
    
    cmdLen = MES_SET_ICON_LEN;

    Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
    Wire.write(outBuff, cmdLen);       // sends device ID and data
    Wire.endTransmission();            // complete transmit

  }  
}

uint8_t makeHeader( uint8_t len )
{
  uint8_t temp;
 
  temp = ((~len) << 4) & 0xF0;    // shift and mask
  temp = temp | (len & 0x0F);
  return (temp);
}

