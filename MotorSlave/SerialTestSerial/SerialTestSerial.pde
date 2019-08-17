/** 
 * Simple Serial Port test GUI
 * 10apr18  ndp
 * revised 29apr19 ndp for Motor Slave
 * 
 * Supports commands
 * a - LED dislay
 * b - MS LED ON
 * c - MS LED OFF
 *
 */


import processing.serial.*;

final byte LED_2x4 = 0x38;
final byte MOTOR_SLAVE = 0x50;

color bgcolor;			     // Background color
color fgcolor;			     // Fill color
Serial myPort;                       // The serial port
char[] serialInArray = new char[64];    // Where we'll put what we receive
int serialCount = 0;                 // A count of how many bytes we receive

int tMills;
String[] eListBox = new String[64];
int eListBoxCount = 0;
PFont mono;

String lTimeBox;

char  eTic = 0;
boolean kFlag = false;

String[] eventList = new String[64];
char[] charKeyBuf = new char[16];

int keyCount = 0;
int[] keyBuf = new int[8];
int tempVal = 0;

void setup() {
  size(600, 900);    // Stage size
  //  noStroke();      // No border on the next thing drawn
  bgcolor = color(50, 50, 50);
  fgcolor = color(255, 255, 255);

  // Print a list of the serial ports, for debugging purposes:
  printArray(Serial.list());

  // I know that the first port in the serial list on my mac
  // is always my  FTDI adaptor, so I open Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  // Use [1] for Windows 10, [0] for Windows 7 it seems.
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 9600);

  tMills = millis() + 1000;

  mono = createFont("Arial", 12);
  textSize(12);
};


void draw() {
  background(bgcolor);
  fill(fgcolor);

  // Update text box
  for (int i=0; i<64; i++) {
    if (eListBox[i] != null) {
      text(eListBox[i], 10, (i*13)+13, 350, (i*26)+26);
    }
  }

  // Display Local Time
  if (lTimeBox != null) {
    text(lTimeBox, 400, 13, 590, 26);
  }
}

// Trigger COMM
// Protocol: REG CMD Data
/*  REG_LED       0xF0 Data  1:ON 2:OFF
 *  REG_MOTOR1    0x10 CMD Data
 *  REG_MOTOR2    0x20 CMD Data
 *  REG_MOTOR3    0x30 CMD Data
 *  REG_MOTOR4    0x40 CMD Data
 * POS 00 UP LP
 * SPS 01 SS
 * DIR 02 0:FWD/1:REV
 * HOM 03
 */
void keyPressed()
{
  boolean flag = false;

  switch(key) {
  case ENTER:
    // send message
    int index = 0;
//    while (keyBuf[index] != 0)
    while( keyCount > 0 )
    {
      myPort.write(keyBuf[index]);
      print(hex(keyBuf[index], 2));
      print(' ');
      ++index;
      keyCount -= 2;
    }
    println();
    String cmd = new String(charKeyBuf);
    println(cmd);
    for(int i = 0; i<8; i++)
    {
      print(hex(keyBuf[i], 2));
      print(' ');
    }
    println();
    
    // flush buffer
    for (int i=0; i<8; i++)
    {
      keyBuf[i] = 0;
    }
    // flush buffer
    for (int i=0; i<16; i++)
    {
      charKeyBuf[i] = 0;
    }
    keyCount = 0;          // reset count
    break;

  default:
    if ( (key >= '0') && (key <= '9') ) {
      // numeric
      tempVal = key - '0';      // convert to 0-9
      flag = true;
    } else if ( (key >= 'A') && (key <= 'F') ) {
      // hex alpha
      tempVal = (key - 'A') + 10;      // convert to 0x0A-0x0F
      flag = true;
    } else if ( (key >= 'a') && (key <= 'f') ) {
      // hex alpha
      tempVal = (key - 'a') + 10;      // convert to 0x0A-0x0F
      flag = true;
    }
    if (flag) {
      // Add to char buff for testing.
      charKeyBuf[keyCount] = key;
      // test for position..0 = HB, 1 = LB
      if (keyCount % 2 == 0) {
        // shift and save
        keyBuf[keyCount >> 1] = tempVal << 4;
      } else {
        // OR into buff
        keyBuf[keyCount >> 1] |= (tempVal & 0x0F);
      }
      ++keyCount;
    }
    break;
  }
}

void serialEvent(Serial myPort) {
  // read a byte from the serial port:
  char inByte = (char)myPort.read();
  // Add the latest byte from the serial port to array:
  serialInArray[serialCount] = inByte;
  serialCount++;

  if (inByte == 10) {
    String sOut = new String(subset(serialInArray, 0, serialCount));
    // print the values (for debugging purposes only):
//    println(sOut);
    eListBox[eListBoxCount] = sOut;
    if (++eListBoxCount > 63) {
      eListBoxCount = 0;

      // TEST
      lTimeBox = nf(second(), 4);    // sOut;
    }

    // Reset serialCount:
    serialCount = 0;
  }
}
