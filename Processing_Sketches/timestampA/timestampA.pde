/**
 * GettingStarted
 *
 * A sketch to list the available serial ports
 * and display characters received
 */


import processing.serial.*;

Serial myPort;      // Create object from Serial class
int portIndex = 0;  // set this to the port connected to Arduino
int val;            // Data received from the serial port

void setup()
{
  size(200, 200);
  println(Serial.list()); // print the list of all the ports
  println(" Connecting to -> " + Serial.list()[portIndex]);
  myPort = new Serial(this, Serial.list()[portIndex], 9600);
  delay(5000);
  String thing = ""+millis();
  myPort.write(thing);
}

String timeStamp() {
  return year()+"/"+month();
}

void draw(){
  while (myPort.available() > 0) {
    delay(200);
    String inBuffer = myPort.readString();   
    if (inBuffer != null) {
      //print(year()+"/"+month()); print("/"); print(day()); print(" ");
      //print(hour()); print(":"); print(minute()); print(":"); print(second()); print(" ");
      //print(inBuffer);
    }
  }
  
}