#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>

#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;

File dataFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if(!dataFile) {
    Serial.println("error opening datalog.txt");
  }

}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "1234abcd";

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.flush();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("File not open. Please try again later.");
    delay(2000);
    return;
  }
  delay(2000);
}
