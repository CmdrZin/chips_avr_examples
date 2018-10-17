/** 
 * RC Interface GUI
 * 17oct18  ndp
 * 
 * Supports commands
 * C  - get Car data - Steering & Throttle
 * D  - 
 * E  - 
 * L  - 
 * R  - 
 * T  - get Temperature
 * S  - 
 * V  - get version and build date
 *
 */
 

import processing.serial.*;

static char SYNC = 0x5A;

color bgcolor;			     // Background color
color fgcolor;			     // Fill color
Serial myPort;                       // The serial port
char[] serialInArray = new char[48];    // Where we'll put what we receive
int serialCount = 0;                 // A count of how many bytes we receive

int tMills;
String[] eListBox = new String[66];
int eListBoxCount = 0;
PFont mono;

String lTimeBox;

char  eTic = 0;
boolean kFlag = false;
boolean ledState = false;

String[] eventList = new String[64];

void setup() {
  size(600, 900);    // Stage size
//  noStroke();      // No border on the next thing drawn
  bgcolor = color(50, 50, 50);
  fgcolor = color(255, 255, 255);

  // Print a list of the serial ports, for debugging purposes:
  printArray(Serial.list());

  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  // Use [1] for Windows 10, [0] for Windows 7 it seems.
  String portName = Serial.list()[1];
  myPort = new Serial(this, portName, 9600);
  
  tMills = millis() + 1000;
  
  mono = createFont("Arial", 12);
  textSize(12);
};

// Called each Frame update
void draw() {
  background(bgcolor);
  fill(fgcolor);
  
  // Update text box
  for(int i=0; i<64; i++) {
    if(eListBox[i] != null) {
      text(eListBox[i], 10, (i*13)+13, 350, (i*26)+26);
    }
  }
  // Trigger COMM
  if(keyPressed && !kFlag) {
    kFlag = true;
    switch(key) {
      case 'c':
        myPort.write(SYNC);
        myPort.write(0x11);
        break;
        
      case 'l':
        myPort.write(SYNC);
        myPort.write(0x00);
        if(ledState) {
          ledState = false;
          myPort.write(0x01);
        } else {
          ledState = true;
          myPort.write(0x00);
        }
        break;
        
      case 't':
        myPort.write(SYNC);
        myPort.write(0x10);
        break;

      case 'v':
        myPort.write('V');
        break;
        
      default:
        break;
    }
  }

  // Display Local Time
  if(lTimeBox != null) {
    text(lTimeBox, 100, 33, 190, 56);
  }

}

// Needed to block key repeates.
void keyReleased()
{
  kFlag = false;
}


void serialEvent(Serial myPort) {
  // read a byte from the serial port:
  char inByte = (char)myPort.read();
    // Add the latest byte from the serial port to array:
    serialInArray[serialCount] = inByte;
    serialCount++;

    if (inByte == 10) {
//      String sOut = new String(subset(serialInArray,0,serialCount));
      
      String sOutH = new String(hex(serialInArray[0],2) + ' ' + hex(serialInArray[1],2));
      
      // print the values (for debugging purposes only):
//      println(sOutH);
//      print(hex(serialInArray[0],2));
//      println(hex(serialInArray[1],2));
      
      eListBox[eListBoxCount] = sOutH;
      if(++eListBoxCount > 63) {
        eListBoxCount = 0;
      }
      eListBox[eListBoxCount] = new String(" ");          // Zero out next display place.

      // TEST
      lTimeBox = sOutH;

      // Reset serialCount:
      serialCount = 0;
    }
}
