#include <FileIO.h>
#include <TimeLib.h>
#include <HX711.h>

//===============Change values as necessary===================
const String stationName = "Test Station";
const String fileName = "DATALOG.txt";
#define SCALE_COUNT 2
#define TIME_BETWEEN_READINGS 500 //time between readings, in milliseconds
#define TIME_BETWEEN_SAVES 5000 //time between saves, in milliseconds
#define TIME_BETWEEN_UPLOADS 10000

int pinsDOUT[SCALE_COUNT] = {8,A0}; 
//The pins hooked up to the respective cells' DOUT

int pinsSCK[SCALE_COUNT] = {9,A1};
//The pins hooked up to the respective cells' SCK

float calibrations[SCALE_COUNT] = {-10000, -10000};
//The calibration factors for the cells

int gain = 128;
//the gain can be 128 or 64 on channel A, or 32 on channel B
//=============================================================

const int chipPin = 4;
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

HX711 *allCells[SCALE_COUNT] = {NULL, NULL}; 

unsigned long prevRead = 0;
unsigned long prevSave = 0;
unsigned long prevUpload = 0;
float cellReadings[SCALE_COUNT] = {0,0};
int readsSinceSave = 0;

//=====================================================================

void setup() {
  setTime(1357041600);
  Serial.begin(9600);
  while (!Serial); // wait for Serial port to connect.
  Serial.println("Filesystem datalogger\n");
  Bridge.begin();
  Serial.println("bridge begun");
  FileSystem.begin();
  Serial.println("filesystem initialized");

  for(int ii=0; ii<SCALE_COUNT; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
  }
  Serial.println("Loadcells initialized");
}

void loop(){    
  //Read each loadcell
  if (millis() - prevRead > TIME_BETWEEN_READINGS) {
    for (int ii=0; ii<SCALE_COUNT; ii++) {
      cellReadings[ii] += allCells[ii]->get_units();
    }
    readsSinceSave++;
    prevRead = millis();
  }

  //average the readings and save to SD card
  if (millis() - prevSave > TIME_BETWEEN_SAVES) {
    
    saveString(makeDataString(cellReadings, &readsSinceSave, stationName), fileName);
    prevSave = millis();
  }
}

//=============================================
//==================FUNCTIONS==================
//=============================================
String dateDisplay(time_t t) {
  String date = String(year(t));
  date += "-";
  date += String(month(t));
  date += "-";
  date += String(day(t));
  return date;
}

String timeDisplay(time_t t) {
  String timer = stringDigits(hour(t));
  timer += ":";
  timer += stringDigits(minute(t));
  timer += ":";
  timer += stringDigits(second(t));
  return timer;
}

String stringDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  String strDigits = "";
  if(digits < 10) {
    strDigits += '0';
  }
  strDigits += String(digits);
  return strDigits;
}

String makeDataString(float *cellReadings,int *readsSinceSave,String stationName) {
  String dataString = "";
  time_t t = now();
  dataString += dateDisplay(t);
  dataString += " ";
  dataString += timeDisplay(t);
  dataString += ",\'"+stationName+"\'"; // insert real station name here
  for (int ii=0; ii<SCALE_COUNT; ii++) {
    dataString += ",";
    dataString += cellReadings[ii] / *readsSinceSave;
    cellReadings[ii] = 0;
  }
  *readsSinceSave = 0;
  return dataString;
}

void saveString(String mystring, String fileName){
  Serial.println(mystring);
  String myFile = "/mnt/sd/" + fileName;
  File dataFile = FileSystem.open(myFile.c_str(), FILE_APPEND);
  dataFile.println(mystring);
  dataFile.close();
}
