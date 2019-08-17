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
byte[] serialInArray = new byte[16];    // Where we'll put what we receive
int serialCount = 0;                 // A count of how many bytes we receive

int tMills;
String[] eListBox = new String[26];
int eListBoxCount = 0;
PFont mono;

String lTimeBox;

char  eTic = 0;
boolean kFlag = false;

String[] eventList = new String[14];

int keyCount = 0;

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
  String portName = Serial.list()[1];
  myPort = new Serial(this, portName, 115200);
  
  tMills = millis() + 1000;
  
  mono = createFont("Arial", 12);
  textSize(12);
};


void draw() {
  background(bgcolor);
  fill(fgcolor);
  
  // Update text box
  for(int i=0; i<26; i++) {
    if(eListBox[i] != null) {
      text(eListBox[i], 10, (i*13)+13, 350, (i*26)+26);
    }
  }
  // Trigger COMM
  if(keyPressed && !kFlag) {
    kFlag = true;
    switch(key) {
      case 'a':
        myPort.write((LED_2x4<<1)|0x00);
        myPort.write(0x05);
        myPort.write(0x01);
        myPort.write(keyCount);
        myPort.write(0x05);
        myPort.write(0x06);
        myPort.write(0x07);
        ++keyCount;
        break;
        
      case 'b':
        myPort.write((MOTOR_SLAVE<<1)|0x00);
        myPort.write(0x02);
        myPort.write(0x01);
        myPort.write(0x01);
        break;
        
      case 'c':
        myPort.write((MOTOR_SLAVE<<1)|0x00);
        myPort.write(0x02);
        myPort.write(0x01);
        myPort.write(0x00);
        break;
        
      case 'd':
        eListBoxCount = 0;
        keyCount = 0;
        break;
  
      // Read operation
      case 'r':
//        myPort.write((0x40<<1)|0x01);
//        myPort.write(0x01);
        break;
        
      default:
        break;
    }
  }

  // Display Local Time
  if(lTimeBox != null) {
    text(lTimeBox, 400, 13, 590, 26);
  }

}

// Needed to block key repeates.
void keyReleased()
{
  kFlag = false;
}


// Triggered on Serial In Received.
void serialEvent(Serial myPort) {

  ++keyCount;

// read a byte from the serial port:
    Integer inByte = new Integer(myPort.read());
    
    // Add the latest byte from the serial port to array:
    serialCount++;

    if (inByte != 0) {
      String sOut = Integer.toString(inByte);
      // print the values (for debugging purposes only):
      println(eListBoxCount + ": " + sOut);      // DEBUG
      eListBox[eListBoxCount] = eListBoxCount + ": " + sOut;
      if(++eListBoxCount > 23) {
        eListBoxCount = 0;
        
        // TEST
        lTimeBox = sOut;
        
      }

      // Reset serialCount:
      serialCount = 0;
    }
}
