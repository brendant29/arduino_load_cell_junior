/**
 * GettingStarted
 *
 * A sketch to list the available serial ports
 * and display characters received
 */


import processing.serial.*;

import java.util.Date;


Serial myPort;      // Create object from Serial class
int portIndex = 0;  // set this to the port connected to Arduino
int val;            // Data received from the serial port

void setup()
{
  size(200, 200);
  println(Serial.list()); // print the list of all the ports
  println(" Connecting to -> " + Serial.list()[portIndex]);
  myPort = new Serial(this, Serial.list()[portIndex], 9600);
  Date d = new Date();
  long current = d.getTime()/1000;
}


void draw(){
  while (myPort.available() > 0) {
    delay(200);
    String inBuffer = myPort.readString();   
    if (inBuffer != null) {
      print(inBuffer);
    }
  }
  
}