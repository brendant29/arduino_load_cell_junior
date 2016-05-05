#include <Fat16.h>
#include <Fat16util.h>

const uint8_t chipPin = 4;
const char *fileName[] = {"DATALOG.txt"};
Fat16 dataFile;
SdCard myCard;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  dataFile = startSDfile(chipPin, fileName);
  Serial.println("ready");
  while (Serial.available() < 1);
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "othertest";

  // if the file is available, write to it:
  if (dataFile.isOpen()) {
    dataFile.println(dataString);
    // print to the serial port too:
    Serial.println(dataString);
  }
   //if the file isn't open, pop up an error:
  else {
    Serial.println("File not open!");
  }
  delay(2000);
}


Fat16 startSDfile(uint8_t chipSelect, const char *filePath[]) {
  if (!myCard.init(true, chipSelect)) {
    //error("Whaaa?");
    Serial.println("No card!");
  }
  Fat16 newFile;
  Fat16::init(&myCard);
  newFile.open(*filePath, O_CREAT | O_RDWR | O_APPEND);
  return newFile;
}

void error(const char* str) {
  Serial.print("error: ");
  Serial.println(str);
  if (myCard.errorCode) {
    Serial.print("SD error: ");
    Serial.println(myCard.errorCode, HEX);
  }
  while(1);
}
