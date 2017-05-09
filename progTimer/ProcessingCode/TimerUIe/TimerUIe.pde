/** 
 * Example GUI
 * 03apr17  ndp
 * 
 * Supports commands
 * C  - Go to PreUse - SelfCheck mode
 * D  - Dump LOG
 * E  - Erase Log
 * L  - send event
 * R  - lost events
 * T  - get local time
 * S  - set local time
 * V  - get version and build date
 *
 * All time is in iso format: 2017-03-25 14:54:23
 */
 

import processing.serial.*;

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

String[] eventList = new String[64];

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
  myPort = new Serial(this, portName, 4800);
  
  tMills = millis() + 1000;
  
  mono = createFont("Arial", 12);
  textSize(12);

  // Load Array..replace this with a read from file operation.
//  for(int i=0; i<10; i++) {
//    eventList[i] = new String("L" + (char)i + "2017-03-11 19:" + (char)('0'+(int)((i*5)/10)) + (char)('0'+((i%2)*5)) + ":0" + (char)('0'+(int)(i % 6)+3));
//  }

  // Preload 0-9
  eventList[0] = new String("L" + (char)0 + "2017-05-08 16:00:03");
  eventList[1] = new String("L" + (char)1 + "2017-05-08 21:00:04");
  eventList[2] = new String("L" + (char)2 + "2017-05-09 16:05:05");
  eventList[3] = new String("L" + (char)3 + "2017-05-09 21:00:06");
  eventList[4] = new String("L" + (char)4 + "2017-05-10 16:00:07");
  eventList[5] = new String("L" + (char)5 + "2017-05-10 21:00:08");
  eventList[6] = new String("L" + (char)6 + "2017-05-11 16:00:09");
  eventList[7] = new String("L" + (char)7 + "2017-05-11 21:00:03");
  eventList[8] = new String("L" + (char)8 + "2017-05-12 16:00:04");
  eventList[9] = new String("L" + (char)9 + "2017-05-12 21:00:05");

  for(int i=10; i<64; i++) {
    eventList[i] = new String("L" + (char)i + "2017-06-11 12:34:56");
  }
  
};


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
        myPort.write('C');
        eListBoxCount = 0;
        break;
        
      case 'd':
        myPort.write('D');
        eListBoxCount = 0;
        break;
        
      case 'e':
        myPort.write('E');
        eListBoxCount = 0;
        break;
        
      case 'l':
//        myPort.write("L"+ eTic + "2017-03-11 19:0" + (char)('0'+eTic++) + ":00");
        for(int i=0; i<64; i++)
        {
           myPort.write(eventList[i]);
           // Delay for O K LF CR response.
           delay(100);
        }
        break;
        
      case 'r':
        myPort.write('R');
        eListBoxCount = 0;
        break;
        
      case 's':
        myPort.write("S2017-05-07 20:35:00");
        break;
        
      case 't':
        myPort.write('T');
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
    text(lTimeBox, 400, 13, 590, 26);
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