/* Arduino Tank Bot Board Ver 2 Avoid Demo Mon (No Serial Out) */

#include <Wire.h>

#define SERIAL_USB  1

/* *** LOCAL PROTOTYPES *** */
uint8_t makeHeader( uint8_t len );
uint8_t objectInRange();   // 1 if obstical
void setMotorSpeed( uint8_t motor, uint16_t mspeed );
void setMotorDirection( uint8_t motor, uint8_t mdirection );
void sendMotorUpdate();

#define  SLAVE_ADRS  0x60        // MUST match AVR chip I2C address

/* NOTE: ALL commands are a minimum of three bytes. HDR MOD CMD .. .. */

/* MOTOR CONTROL */
// Set to match DC motor assignments
#define MDM_LEFT   1
#define MDM_RIGHT  2
#define MDM_BOTH   3

#define MDM_STOP    0
#define MDM_FORWARD 1
#define MDM_REVERSE 2

#define MDM_LEFT_SLOW   5000
#define MDM_LEFT_MEDIUM 10000
#define MDM_LEFT_FAST   19000

#define MDM_RIGHT_SLOW   5000
#define MDM_RIGHT_MEDIUM 10000
#define MDM_RIGHT_FAST   19000

#define MOD_DC_MOTOR_ID  0x30
#define MDM_SET_SPEED           1
#define MDM_SET_SPEED_LEN       6

#define MDM_SET_DIRECTION       2
#define MDM_SET_DIRECTION_LEN   5

#define MDM_UPDATE              3
#define MDM_UPDATE_LEN          3

typedef enum { AD_IDLE, AD_FORWARD_1, AD_FORWARD_2, AD_STOP, AD_BACKUP, AD_TURN_AROUND } AD_STATE;

/* Ultra-sonic Range Sensors */
#define MOD_RANGE_FINDER_US_ID  0x20
#define MRFUS_GET_RANGE           1
#define MRFUS_GET_RANGE_LEN       4
#define MRFUS_GET_RANGE_DATA_LEN  2

#define MRFUS_GET_ALL_RANGE       2
#define MRFUS_GET_ALL_RANGE_LEN   3
#define MRFUS_GET_ALL_RANGE_DATA_LEN  4

#define MRFUS_SET_MIN_RANGE       3
#define MRFUS_SET_MIN_RANGE_LEN   5

#define MRFUS_MIN_RANGE  30          // N * cm  (10 = 5 inches)

/* GLOBALS */
int slave = SLAVE_ADRS;   // has to be an int.
byte count = 0;           // simple counter
int cmdLen;
uint8_t outBuff[8];
AD_STATE demoState;

void setup()
{
  Wire.begin();             // join i2c bus
  demoState = AD_IDLE;
}

/* AVOID Demo */
void loop()
{
  switch( demoState )
  {
    // Initialize
    case AD_IDLE:
      setMotorDirection( MDM_BOTH, MDM_STOP );
      setMotorSpeed( MDM_BOTH, MDM_STOP );
      demoState = AD_FORWARD_1;
      break;

    case AD_FORWARD_1:
      setMotorDirection( MDM_BOTH, MDM_FORWARD );
      setMotorSpeed( MDM_BOTH, MDM_RIGHT_MEDIUM );    // TODO: Calibrate l/R speeds
      count = 10;                                     // wait 1 second before sending update.
      demoState = AD_FORWARD_2;
      break;

    // Check for obsticals while moving forward.
    case AD_FORWARD_2:
      if( objectInRange() ) {
        setMotorDirection( MDM_BOTH, MDM_STOP );
        setMotorSpeed( MDM_BOTH, MDM_STOP );
        demoState = AD_STOP;
      } else {
        if( --count == 0 ) {
          sendMotorUpdate();        // Keep going
          count = 10;                                 // reset
        }
      }
      break;
    
    case AD_STOP:
      // Start backing up.
      setMotorDirection( MDM_BOTH, MDM_REVERSE );
      setMotorSpeed( MDM_BOTH, MDM_RIGHT_MEDIUM );
      demoState = AD_BACKUP;
      count = 10;                                     // backup for 1.
      break;
      
    case AD_BACKUP:
      // Wait for backup time then turn around.
      if( --count == 0 ) {
        // Start turning around
        setMotorDirection( MDM_RIGHT, MDM_FORWARD );
        setMotorSpeed( MDM_BOTH, MDM_RIGHT_MEDIUM );
        demoState = AD_TURN_AROUND;
        count = 30;                                     // turn around for 3 seconds.
      }
      // Keep updating as needed.
      if( count % 10 == 0 ) {
        sendMotorUpdate();
      }
      break;

    case AD_TURN_AROUND:
      // Wait for turn around time then go back to moving forward.
      if( --count == 0 ) {
        setMotorDirection( MDM_BOTH, MDM_FORWARD );
        setMotorSpeed( MDM_BOTH, MDM_RIGHT_MEDIUM );
        demoState = AD_FORWARD_1;
      }
      // Keep updating as needed.
      if( count % 10 == 0 ) {
        sendMotorUpdate();
      }
      break;
  }

  delay(100);   // 100 ms
}

/*
 * Check range sensors.
 * Return true if obstical
 */
uint8_t objectInRange()
{
  uint8_t dataBuf[3];
  uint8_t index;
  uint8_t data_len;
  uint8_t i;

  // preload with max values for range test.
  for( i=0; i<sizeof(dataBuf); i++ ) {
    dataBuf[i] = 0xff;
  }
  
#if 0
  // Returns ID D1
  outBuff[0] = makeHeader( MRFUS_GET_RANGE_LEN-3 );
  outBuff[1] = MOD_RANGE_FINDER_US_ID;
  outBuff[2] = MRFUS_GET_RANGE;
  outBuff[3] = 1;             // CENTER
    
  cmdLen = MRFUS_GET_RANGE_LEN;
  data_len = MRFUS_GET_RANGE_DATA_LEN;
#endif

#if 1
  // Returns ID D1 D2 D3
  outBuff[0] = makeHeader( MRFUS_GET_ALL_RANGE_LEN-3 );
  outBuff[1] = MOD_RANGE_FINDER_US_ID;
  outBuff[2] = MRFUS_GET_ALL_RANGE;
  outBuff[3] = 0;
    
  cmdLen = MRFUS_GET_ALL_RANGE_LEN;
  data_len = MRFUS_GET_ALL_RANGE_DATA_LEN;
#endif
  
  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.

  // Issues a SLA_R request and triggers N reads before NACKing Slave.
  Wire.requestFrom(slave, data_len);    // request N bytes from (int)slave

  // Flush buffer
  while(!Wire.available());          // wait for DEV number.
  unsigned char d = Wire.read();     // Skip DEV number

  i = 0;
  while( Wire.available() && (i < sizeof(dataBuf)) )  // slave may send less than requested
  { 
    dataBuf[i] = Wire.read();        // receive a byte as character
    ++i;
  }

  for( i=0; i<sizeof(dataBuf); i++ ) {
    if( dataBuf[i] < MRFUS_MIN_RANGE ) return(1);
  }

  return(0);
}

/*
 * Set speed for Left, Right, or Both
 * mspeed = 0:20000
 */
void setMotorSpeed( uint8_t motor, uint16_t mspeed )
{
  outBuff[0] = makeHeader( MDM_SET_SPEED_LEN-3 );
  outBuff[1] = MOD_DC_MOTOR_ID;
  outBuff[2] = MDM_SET_SPEED;
  outBuff[3] = motor;
  // Max speed is 20000
  outBuff[4] = mspeed & 0x00FF;
  outBuff[5] = mspeed >> 8;
   
  cmdLen = MDM_SET_SPEED_LEN;

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
}

/*
 * Set direction for Left, Right, or Both
 * 0=STOP, 1=Forwared, 2=Reverse
 */
void setMotorDirection( uint8_t motor, uint8_t mdirection )
{
  outBuff[0] = makeHeader( MDM_SET_DIRECTION_LEN-3 );
  outBuff[1] = MOD_DC_MOTOR_ID;
  outBuff[2] = MDM_SET_DIRECTION;
  outBuff[3] = motor;
  outBuff[4] = mdirection;
 
  cmdLen = MDM_SET_DIRECTION_LEN;

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
}

/*
 * Send control update to keep motors on.
 */
void sendMotorUpdate()
{
  outBuff[0] = makeHeader( MDM_UPDATE_LEN-3 );
  outBuff[1] = MOD_DC_MOTOR_ID;
  outBuff[2] = MDM_UPDATE;
 
  cmdLen = MDM_UPDATE_LEN;

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
}

uint8_t makeHeader( uint8_t len )
{
  uint8_t temp;
 
  temp = ((~len) << 4) & 0xF0;    // shift and mask
  temp = temp | (len & 0x0F);
  return (temp);
}

