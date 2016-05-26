#include <HX711.h>
#include <FileIO.h>

#define SCALE_COUNT 1

int pinsDOUT[SCALE_COUNT] = {8}; 
//The pins hooked up to the respective cells' DOUT

int pinsSCK[SCALE_COUNT] = {9};
//The pins hooked up to the respective cells' SCK

float calibrations[SCALE_COUNT] = {-10000};
//The calibration factors for the cells

HX711 *allCells[SCALE_COUNT] = {NULL}; 

void setup() {
  // Initialize the Bridge and the Serial
  Serial.begin(9600);
  while (!SerialUSB); // wait for Serial port to connect.
  SerialUSB.println("Filesystem datalogger\n");
  Bridge.begin();
  SerialUSB.println("bridge begun");
  FileSystem.begin();
  SerialUSB.println("filesystem initialized");

  for(int ii=0; ii<SCALE_COUNT; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
  }
  SerialUSB.println("Loadcells initialized");
}


void loop() {
  // make a string that start with a timestamp for assembling the data to log:
  String dataString;
  dataString += getTimeStamp();
  dataString += " = ";

  for(int ii=0; ii<SCALE_COUNT; ii++){
    dataString += (",");
    dataString += (allCells[ii]->get_units()); //the actual reading
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // The FileSystem card is mounted at the following "/mnt/FileSystema1"
  File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
  }
  // if the file isn't open, pop up an error:
  else {
    SerialUSB.println("error opening datalog.txt");
  }
  SerialUSB.println(dataString);

  delay(5000);

}

// This function return a string with the time stamp
String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  time.begin("date");
  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
  //             T for the time hh:mm:ss
  time.run();  // run the command

  // read the output of the command
  while (time.available() > 0) {
    char c = time.read();
    if (c != '\n') {
      result += c;
    }
  }

  return result;
}
