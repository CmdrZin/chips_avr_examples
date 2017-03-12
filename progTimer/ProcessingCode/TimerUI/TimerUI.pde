/** 
 * Example GUI
 * 03mar17  ndp
 * 
 * Supports commands
 * L  - send event
 * R  - lost events
 * T  - get local time
 * S  - set local time
 *
 * All time is in iso format: 2017-03-25 14:54:23
 */
 

import processing.serial.*;

color bgcolor;			     // Background color
color fgcolor;			     // Fill color
Serial myPort;                       // The serial port
char[] serialInArray = new char[32];    // Where we'll put what we receive
int serialCount = 0;                 // A count of how many bytes we receive

int tMills;
String[] eListBox = new String[66];
int eListBoxCount = 0;
PFont mono;

String lTimeBox;

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
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 4800);
  
  tMills = millis() + 1000;
  
  mono = createFont("Arial", 12);
  textSize(12);
}

void draw() {
  background(bgcolor);
  fill(fgcolor);
  
  // Update text box
  for(int i=0; i<64; i++) {
    if(eListBox[i] != null) {
      text(eListBox[i], 10, (i*13)+13, 200, (i*26)+26);
    }
  }
  // Trigger COMM
  if( tMills < millis() ) {
    // Send a T read current time.
    // Send a R to read Event list.
      myPort.write('R');
      tMills = millis() + 5000;
  }

  // Display Local Time
  if(lTimeBox != null) {
    text(lTimeBox, 200, 13, 300, 26);
  }

}

void serialEvent(Serial myPort) {
  // read a byte from the serial port:
  char inByte = (char)myPort.read();
    // Add the latest byte from the serial port to array:
    serialInArray[serialCount] = inByte;
    serialCount++;

    if (inByte == 13) {
      String sOut = new String(subset(serialInArray,0,serialCount-1));
      // print the values (for debugging purposes only):
//      println(sOut);
      eListBox[eListBoxCount] = sOut;
      if(++eListBoxCount > 63) {
        eListBoxCount = 0;
        
        // TEST
        lTimeBox = sOut;
        
      }

      // Reset serialCount:
      serialCount = 0;
    }
}