#include <SPI.h>
#include <SD.h>
#include <TimeLib.h>
#include <HX711.h>

const int chipPin = 4;
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

//===============Change values as necessary===================
const String fileName = "DATALOG.txt";
#define SCALE_COUNT 4
#define TIME_BETWEEN_READINGS 500 //time between readings, in milliseconds
#define TIME_BETWEEN_SAVES 2000 //time between saves, in milliseconds

int pinsDOUT[SCALE_COUNT] = {3,5,7,9}; 
//The pins hooked up to the respective cells' DOUT

int pinsSCK[SCALE_COUNT] = {2,4,6,8};
//The pins hooked up to the respective cells' SCK

float calibrations[SCALE_COUNT] = {-10000, -10000, -10000, -10000};
//The calibration factors for the cells

int gain = 128;
//the gain can be 128 or 64 on channel A, or 32 on channel B
//=============================================================

HX711 *allCells[SCALE_COUNT] = {NULL, NULL, NULL, NULL}; 

unsigned long prevRead = 0;
unsigned long prevSave = 0;
float cellReadings[SCALE_COUNT] = {0,0,0,0};
int readsSinceSave = 0;
File dataFile;

void setup()  {
  Serial.begin(9600);
  //pinMode(13, OUTPUT);
  setSyncProvider( requestSync);  //set function to call when sync required
  
  dataFile = startSDfile(chipPin, fileName);
  
  //setting up the cells
  for(int ii=0; ii<SCALE_COUNT; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->set_gain(gain);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
  }
  delay(200);
}

void loop(){    
  if (Serial.available()) {
    processSyncMessage();
  }
  
  if (millis() - prevRead > TIME_BETWEEN_READINGS) {
    for (int ii=0; ii<SCALE_COUNT; ii++) {
      cellReadings[ii] += allCells[ii]->get_units();
    }
    readsSinceSave++;
    prevRead = millis();
  }
  
  if (millis() - prevSave > TIME_BETWEEN_SAVES) {
    
    saveString(makeDataString(cellReadings, &readsSinceSave), dataFile);
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
  String timer = String(hour(t));
  timer += stringDigits(minute(t));
  timer += stringDigits(second(t));
  return timer;
}

String stringDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  String strDigits = ":";
  if(digits < 10) {
    strDigits += '0';
  }
  strDigits += String(digits);
  return strDigits;
}

String makeDataString(float *cellReadings,int *readsSinceSave) {
  String dataString = "";
  time_t t = now();
  dataString += dateDisplay(t);
  dataString += ",";
  dataString += timeDisplay(t);
  for (int ii=0; ii<SCALE_COUNT; ii++) {
    dataString += ",";
    dataString += cellReadings[ii] / *readsSinceSave;
    cellReadings[ii] = 0;
  }
  *readsSinceSave = 0;
  return dataString;
}

void saveString(String mystring, File myFile){
  Serial.println(mystring);
  myFile.println(mystring);
  myFile.flush();
}

File startSDfile(int chipSelect, String filePath) {
  if (!SD.begin(chipSelect)) {
    
  }
  File newFile = SD.open(filePath, FILE_WRITE);
  return newFile;
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
     }
  }
}

time_t requestSync()
{
  //Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}

