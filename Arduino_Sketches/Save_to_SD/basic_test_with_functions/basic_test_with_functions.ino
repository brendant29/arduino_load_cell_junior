#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>

#include <SPI.h>
#include <SD.h>

const int chipPin = 4;
const String fileName = "DATALOG.txt";
File dataFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  dataFile = startSDfile(chipPin, fileName);

}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "othertest";

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.flush();
    // print to the serial port too:
    Serial.println(dataString);
  }
   //if the file isn't open, pop up an error:
  else {
    Serial.println("nope, no card!");
  }
  delay(2000);
}


File startSDfile(int chipSelect, String filePath) {
  if (!SD.begin(chipSelect)) {
    Serial.println("I don't think the card's available.");
  }
  File newFile = SD.open(filePath, FILE_WRITE);
  return newFile;
}
