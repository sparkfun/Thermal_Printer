/*
 SparkFun Electronics 2011
 Updated By: Sam Wizer 8/17/2018
 Written by: Nathan Seidle 3-2-2011
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 To use this example code, attach
 Arduino : Printer
 D2 : Green wire (Printer TX)
 D3 : Yellow wire (Printer RX)
 VIN (9V wall adapter) : Red wire
 GND : Black wires
 Open Arduino serial window at 38400bps. Turn on line endings!
 
 Looking at the rear of the unit, the left connector is for power (red/black cable). The right connector is 
 for serial communication. The middle pin on the right connector is yellow and is the pin to transmit serial 
 data into. The green pin on the end of the right connector is what serial comes out of.
 
 The SparkFun Thermal Printer takes 2.25" (57mm) wide thermal paper with max roll diameter of 1.5" (39mm). 
 2.25" wide paper x 85ft is commonly found in most office supply stores. I paid $14 for 9 rolls of 85ft per 
 roll ($0.0015 per inch!!), and much cheaper online in bulk. Ask to see their thermal paper, or calculator replacement 
 paper. Most of the rolls sold have too much paper on the spool to fit into the printer. I had to remove about 
 1/3rd of the roll to get it to fit. The orientation of how the spool should fit into the printer is not obvious. 
 Run the printer in demo mode to verify that the printer is able to print. If you get blank paper spitting out, 
 try flipping the paper upside down.
 
 The thermal printer ships with default 19200bps baud rate. To discover the baud rate, hold down the 
 feed button, then attach the printer to the Vin pin on the Arduino. You'll need more current than a USB connection is
 capable of. The printer should go into demo mode. The final information after the Character Table (ABCDEFG), 
 is the baudrate and firmware information. 
 
 A 5V wall adapter worked fine.
 A 5V USB connection was not enough power to allow the roller to advance the paper.
 A 9V source caused the printer to run very fast, but did not complete the demo mode.
 
 Unit uses 4.8mA @ 9V in standby.
 Unit uses about 1.3A! @ 9V during printing.
 
 Heat time controls how hot the head gets on any given line. The larger the number, the darker the text. But if
 you set this too high, with too high a supply voltage, the printer will freeze.
 
 Heat interval controls the paper advancement. The larger the number, the slow the paper advances. This 
 allows a high voltage supply.
 
 Here are settings I found:
 9V adapter: Heat time = 255, heat interval = 255
 5V adapter: Heat time = 255, heat interval = 5
 You can see the heat interval is tied to the power supply. Increasing the printDensity and printBreakTime
 also tended to increase the color darkness.
 
 */
#include <SoftwareSerial.h>
SoftwareSerial Thermal(2, 3); //Soft RX from printer on D2, soft TX out to printer on D3

#define FALSE  0
#define TRUE  1
int printOnBlack = FALSE;
int printUpSideDown = FALSE;

int ledPin = 13;
int heatTime = 255; //80 is default from page 23 of datasheet. Controls speed of printing and darkness
int heatInterval = 255; //2 is default from page 23 of datasheet. Controls speed of printing and darkness
char printDensity = 15; //Not sure what the defaut is. Testing shows the max helps darken text. From page 23.
char printBreakTime = 15; //Not sure what the defaut is. Testing shows the max helps darken text. From page 23.

void setup() {
  pinMode(ledPin, OUTPUT);

  Serial.begin(38400); //Use hardware serial for debugging
  Thermal.begin(19200); //Setup soft serial for ThermalPrinter control

  printOnBlack = FALSE;
  printUpSideDown = FALSE;

  //Modify the print speed and heat
  Thermal.write(27);
  Thermal.write(55);
  Thermal.write(7); //Default 64 dots = 8*('7'+1)
  Thermal.write(heatTime); //Default 80 or 800us
  Thermal.write(heatInterval); //Default 2 or 20us

  //Modify the print density and timeout
  Thermal.write(18);
  Thermal.write(35);
  int printSetting = (printDensity<<4) | printBreakTime;
  Thermal.write(printSetting); //Combination of printDensity and printBreakTime

  Serial.println();
  Serial.println("Parameters set");
}

void loop() {
  char option;

  while(1) {
    Serial.println();
    Serial.println("Thermal Printer Testing");
    Serial.println("1) Print Hello World!");
    Serial.println("2) Type to print");
    Serial.println("3) Dull boy");
    Serial.println("4) Printer status");
    Serial.println("5) Reverse white/black");
    Serial.println("6) Print characters upside down");
    Serial.println("7) Adjust heat settings");
    Serial.println("8) Adjust print settings");
    Serial.println("9) Print factory test page");
    Serial.println("a) Print entire character set");
    Serial.println("b) Print numeric barcode");
    Serial.println("c) Print alpha barcode");
    Serial.println("d) Bitmap test");
    Serial.println("e) Print QR Code");
    Serial.print(": ");

    while(1) {
      while(!Serial.available());
      option = Serial.read();
      if(isalnum(option)) break;
    }

    if(option == '1') {
      Serial.println("Hello World!");

      Thermal.println("Hello World!");
      Thermal.println("12345678901234567890123456789012");
      Thermal.write(10); //Sends the LF to the printer, advances the paper
      Thermal.write(10);
    }
    else if (option == '2') {
      Serial.println("Turn on line endings! Press $ to exit. Tell me what's on your mind.");
      //Free mode
      while(1) {
        while(Serial.available() == 0) ; //Wait for incoming characters

        while(Serial.available() > 0) {
          option = Serial.read(); //Take a character from serial port and check what it is

          if(option == '$') 
            break; //Break if we see the money
          else if(option == 13) 
            Thermal.write(10); //Line feed on CR
          else {
            Thermal.print(option); //Push this character to printer
            Serial.print(option);
            delay(10);
          }
        }

        if(option == '$') break; //Break if we see ctrl+z
      }
    }
    else if (option == '3') {
      //The Shining
      for(int x = 0 ; x < 15 ; x++)
        Thermal.println("All work and no play makes Jack a dull boy");
      Thermal.write(10);
      Thermal.write(10);
      Thermal.write(10);
    }
    else if(option == '4')  {
      Serial.println("Printer Status:");
      Thermal.write(16); 
      Thermal.write(4); 
      Thermal.write(4);  //paper status
      char paperStatus = Thermal.read();
      if (paperStatus & 8){           //Check bit 3
        Serial.println("Paper near end of roll");
      }
      else if (paperStatus & 64){     //Check bit 6
        Serial.println("Refill paper");
      }
      else{
        Serial.println("Plenty of paper!");
      }

      Thermal.write(16); 
      Thermal.write(4); 
      Thermal.write(3);  //transmit error status
      char transmitErrorStatus = Thermal.read();
      if (transmitErrorStatus & 32){     //Check bit 5
        Serial.println("Unrecoverable error!");
      }
      if (transmitErrorStatus & 64){  //Check bit 6
        Serial.println("Print head temp and voltage exceeded");
      }
      
      Thermal.write(16); 
      Thermal.write(4); 
      Thermal.write(2);  //transit offline status
      char transitOfflineStatus = Thermal.read();
      if (transitOfflineStatus & 4){    //Check bit 2
        Serial.println("Cover open");
      }
      if (transitOfflineStatus & 64){    //Check bit 6
        Serial.println("Error");
      }
    }
    else if(option == '5') {
      //Select background color
      Thermal.write(29); //GS-ESC
      Thermal.write(66); //B

      if(printOnBlack == FALSE) {
        printOnBlack = TRUE;
        Serial.println("Print on black");
        Thermal.write(1); //Prints text on black background
      }
      else {
        printOnBlack = FALSE;
        Serial.println("Print on white");
        Thermal.print((byte)0); //Prints text on white background
      }
    }
    else if(option == '6') {
      //Select text orientation
      Thermal.write(27); //ESC
      Thermal.write(123); //{

      if(printUpSideDown == FALSE) {
        printUpSideDown = TRUE;
        Serial.println("Print upside down");
        Thermal.write(1); //Prints upside down
      }
      else {
        printUpSideDown = FALSE;
        Serial.println("Print normal upside");
        Thermal.write((byte)0); //Prints normal
      }
    }

    else if(option == '7') {
      //Change settings
      Serial.println();
      Serial.println("Turn on Newline endings!");

      char buffer[10];
      while(1) {
        Serial.print("Current heat time:");
        Serial.print(heatTime, DEC);
        Serial.print(" New heat time (default is 80):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        heatTime = atoi(buffer); //Convert response to integer
        if(heatTime < 256 && heatTime > 3) break;
      }

      while(1) {
        Serial.print("Current heat interval:");
        Serial.print(heatInterval, DEC);
        Serial.print(" New heat interval (default is 2):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        heatInterval = atoi(buffer); //Convert response to integer
        if(heatInterval < 256 && heatInterval > 2) break;
      }

      //Control Parameters
      Thermal.write(27);
      Thermal.write(55);
      Thermal.write(7); //Default 64 dots = 8*('7'+1)
      Thermal.write((byte)heatTime); //Default 80 or 800us
      Thermal.write((byte)heatInterval); //Default 2 or 20us

      Serial.println();
      Serial.println("Parameters set");
    }
    else if (option == '8') {
      //Change more settings
      Serial.println();
      Serial.println("Turn on Newline endings!");

      char buffer[10];
      while(1) {
        Serial.print("Current print density:");
        Serial.print(printDensity, DEC);
        Serial.print(" New print density (max is 15):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        printDensity = atoi(buffer); //Convert response to integer
        if(printDensity < 16 && printDensity > 0) break;
      }

      while(1) {
        Serial.print("Current print break time:");
        Serial.print(printBreakTime, DEC);
        Serial.print(" New print break time (max is 15):");

        read_line(buffer, sizeof(buffer)); //Read user's response
        printBreakTime = atoi(buffer); //Convert response to integer
        if(printBreakTime < 16 && printBreakTime > 0) break;
      }

      int printSetting = printDensity << 4;
      printSetting |= printBreakTime;

      //Control Parameters
      Thermal.write(18); //DC2
      Thermal.write(35); //#
      Thermal.write(printSetting);

      Serial.println();
      Serial.println("Parameters set");
    }
    else if (option == '9') {
      //Print test page
      Thermal.write(27);  //1B
      Thermal.write(64);  //40
      Thermal.write(18); //DC2
      Thermal.write(84); //T
    }
    else if (option == 'a') {
      //Print the entire character set
      for(int x = 0 ; x < 256 ; x++)
        Thermal.write(x);
      Thermal.write(10);
      Thermal.write(10);
    }
    else if (option == 'b') {
      Serial.println("Barcode printing");
      //Print barcode example
      
      Thermal.write(29); //GS
      Thermal.write(107); //k
      Thermal.write((byte)0); //m = 0
      Thermal.print('2'); //Data
      Thermal.print('9');
      Thermal.print('1');
      Thermal.print('9');
      Thermal.print('3');
      Thermal.print('9');
      Thermal.print('1');
      Thermal.print('9');
      Thermal.print('3');
      Thermal.print('9');
      Thermal.print('1');
      Thermal.print('9'); //12 characters
      Thermal.write((byte)0); //Terminator

      delay(3000); //For some reason we can't immediately have line feeds here
      Thermal.write(10); //Paper feed
      Thermal.write(10); //Paper feed
    }
    else if (option == 'c') {
      //Print barcode example
      Thermal.write(29); //GS
      Thermal.write(107); //k
      Thermal.write(4); //m = 4, fancy bar code
      Thermal.print('W'); //Data, cap letters, $, ., % acceptable
      Thermal.print('W');
      Thermal.print('W');
      Thermal.print('.');
      Thermal.print('S');
      Thermal.print('P');
      Thermal.print('A');
      Thermal.print('R');
      Thermal.print('K');
      Thermal.print('F');
      Thermal.print('U');
      Thermal.print('N');
      Thermal.print('.');
      Thermal.print('C');
      Thermal.print('O');
      Thermal.print('M'); //16 characters
      Thermal.write((byte)0); //Terminator

      delay(3000); //For some reason we can't immediately have line feeds here
      Thermal.write(10); //Paper feed
      Thermal.write(10); //Paper feed
    }
    else if(option == 'd')  {
      //Bitmap example
      Serial.println("Print bitmap image");
      Thermal.write(18); //DC2
      Thermal.write(86); //V
      Thermal.write(50); //nL = 10
      Thermal.write((byte)0); //nH = 0

      //Now we need 10 rows
      for(int y = 0 ; y < 50 ; y++) {
        //Now we need 48 bytes total
        for(int x = 0 ; x < 48 ; x++)
          Thermal.write(0xAA); //0b.1010.1010
      }

      Thermal.write(10); //Paper feed
      Thermal.write(10); //Paper feed

      Serial.println("Print bitmap done");
    }
    else if(option == 'e'){
      //QR code example
      Serial.println("Print QR code");
      Thermal.write(29);  //GS
      Thermal.write(107);  //k
      Thermal.write(97); //m 
      Thermal.write(0x08);
      Thermal.write(0x02);
      Thermal.write(0x08);
      Thermal.write((byte)0x00);

      //Start of data
      Thermal.print('1'); //Data, cap letters, $, ., % acceptable
      Thermal.print('1');
      Thermal.print('2');
      Thermal.print('3');
      Thermal.print('5');
      Thermal.print('8');

      Thermal.write((byte)0); //Terminator

      delay(3000); //For some reason we can't immediately have line feeds here
      Thermal.write(10); //Paper feed
      Thermal.write(10); //Paper feed
    }
    else {
      Serial.print("Choice = ");
      Serial.println(option, DEC);
    }
  }
}

//Reads a line until the \n enter character is found
uint8_t read_line(char* buffer, uint8_t buffer_length) {
  memset(buffer, 0, buffer_length);

  while(Serial.available()) Serial.read(); //Clear out any characters that may be sitting in the buffer

  uint8_t read_length = 0;
  while(read_length < buffer_length - 1) {
    while (!Serial.available()); //We've destroyed the standard way Arduino reads characters so this no longer works
    uint8_t c = Serial.read();

    if(c == 0x08 || c == 0x7f) { //Backspace characters
      if(read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0';

      Serial.print((char)0x08);
      Serial.print(' ');
      Serial.print((char)0x08);

      continue;
    }

    Serial.print((char)c);

    if(c == '\n' || c == '\r') {
      buffer[read_length] = '\0';
      break;
    }
    else {
      buffer[read_length] = c;
      ++read_length;
    }
  }

  return read_length;
}
