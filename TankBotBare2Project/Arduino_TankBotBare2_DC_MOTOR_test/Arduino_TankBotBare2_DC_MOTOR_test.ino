/* Arduino Tank Bot Board Ver 2 DC Motor Test */

#include <Wire.h>

#define SERIAL_USB  1

/* *** LOCAL PROTOTYPES *** */
uint8_t makeHeader( uint8_t len );

#define  SLAVE_ADRS  0x60        // MUST match AVR chip I2C address

/* NOTE: ALL commands are a minimum of three bytes. LEN MOD CMD .. .. */

// Set to match DC motor assignments
#define MDM_LEFT   1
#define MDM_RIGHT  2
#define MDM_BOTH   3

#define MDM_STOP    0
#define MDM_FORWARD 1
#define MDM_REVERSE 2

#define MOD_DC_MOTOR_ID  0x30
#define MDM_SET_SPEED           1
#define MDM_SET_SPEED_LEN       6

#define MDM_SET_DIRECTION       2
#define MDM_SET_DIRECTION_LEN   5

#define MDM_UPDATE              3
#define MDM_UPDATE_LEN          3


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
uint8_t outBuff[8];

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
  outBuff[0] = makeHeader( MDM_SET_SPEED_LEN-3 );
  outBuff[1] = MOD_DC_MOTOR_ID;
  outBuff[2] = MDM_SET_SPEED;
  outBuff[3] = MDM_BOTH;
  // Max speed is 20000
  outBuff[4] = 0;
  outBuff[5] = 20;      // 10 * 256 = 2560
   
  cmdLen = MDM_SET_SPEED_LEN;

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
  Serial.println(outBuff[5], HEX);
#endif

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.
#endif

#if 1
  outBuff[0] = makeHeader( MDM_SET_DIRECTION_LEN-3 );
  outBuff[1] = MOD_DC_MOTOR_ID;
  outBuff[2] = MDM_SET_DIRECTION;
  outBuff[3] = MDM_BOTH;
  if(count & 0x04) {
    outBuff[4] = MDM_REVERSE;
  } else {
    outBuff[4] = MDM_FORWARD;
  }
  
  if(count & 0x08) {
    outBuff[4] = MDM_STOP;
  }
 
  cmdLen = MDM_SET_DIRECTION_LEN;

#if(SERIAL_USB == 1)
  Serial.print(outBuff[0], HEX);
  Serial.print(" ");
  Serial.print(outBuff[1], HEX);
  Serial.print(" ");
  Serial.print(outBuff[2], HEX);
  Serial.print(" ");
  Serial.print(outBuff[3], HEX);
  Serial.print(" ");
  Serial.println(outBuff[4], HEX);
#endif

  Wire.beginTransmission(slave);     // transmit to Slave. Have to do this each time.
  Wire.write(outBuff, cmdLen);       // sends device ID and data
  Wire.endTransmission();            // complete transmit
  delay(1);                          // need to give Slave time to fill ouput FIFO if needed.

#endif

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

uint8_t makeHeader( uint8_t len )
{
  uint8_t temp;
 
  temp = ((~len) << 4) & 0xF0;    // shift and mask
  temp = temp | (len & 0x0F);
  return (temp);
}

