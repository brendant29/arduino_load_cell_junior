#include <Adafruit_CC3000.h>

//#include <SD.h>
#include <TimeLib.h>
#include <HX711.h>

//===============Change values as necessary===================
const String stationName = "Test Station";
const String fileName = "DATALOG.txt";
#define SCALE_COUNT 2
#define TIME_BETWEEN_READINGS 50 //time between readings, in milliseconds
#define TIME_BETWEEN_SAVES 500 //time between saves, in milliseconds
#define TIME_BETWEEN_UPLOADS 10000

int pinsDOUT[SCALE_COUNT] = {7,A0}; 
//The pins hooked up to the respective cells' DOUT

int pinsSCK[SCALE_COUNT] = {6,A1};
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
//File dataFile;

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "The Squishy of Rick"           // cannot be longer than 32 characters!
#define WLAN_PASS       "brendane"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to post to!
#define WEBSITE      "winter-runoff.soils.wisc.edu"

uint32_t ip;
Adafruit_CC3000_Client client;

//=====================================================================

void setup() {
  Serial.begin(9600);
//  setSyncProvider( requestSync);  //set function to call when sync required
  
  //dataFile = startSDfile(chipPin, fileName);
  
  //setting up the cells
  for(int ii=0; ii<SCALE_COUNT; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->set_gain(gain);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
  }
  
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  
}

void loop(){    
  if (Serial.available()) {
    processSyncMessage();
  }

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
    
    saveString(makeDataString(cellReadings, &readsSinceSave, stationName), /*dataFile*/);
    prevSave = millis();
  }

  //Upload to web server
  if (millis() - prevUpload > TIME_BETWEEN_UPLOADS) {
    
  }
  
}

//=============================================
//==================FUNCTIONS==================
//=============================================
String dateDisplay(time_t t) {
  String date ;//= String(year(t));
  date += "-";
  date += String(month(t));
  date += "-";
  date += String(day(t));
  return date;
}

String timeDisplay(time_t t) {
  String timer ;//= stringDigits(hour(t));
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

void saveString(String mystring, /*File myFile*/){
  Serial.println(mystring);
  //myFile.println(mystring);
  //myFile.flush();
}
/*
bool uploadStrings(String mystrings[]) {
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    return false;
  }
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }
  
  for (int ii = 0; ii < 4; ii++) {
    if (!client.connected()) {
      client = connectToServer();
   }
  }
}
*/
void postString(String data,Adafruit_CC3000_Client client) {
  data = "csv_line="+data;
  char datachars[data.length()+1];
  data.toCharArray(datachars,data.length());
  if (client.connected()) { 
    client.fastrprintln(F("POST /time_series_data/upload HTTP/1.1")); 
    client.fastrprint(F("Host: ")); 
    client.fastrprintln(WEBSITE);// SERVER ADDRESS HERE TOO
    client.fastrprintln(F("Content-Type: application/x-www-form-urlencoded")); 
    client.fastrprint(F("Content-Length: ")); 
    client.println(data.length()); 
    client.println(); 
    client.fastrprint(datachars); 
  } 
}

Adafruit_CC3000_Client connectToServer() {
  cc3000.getHostByName(WEBSITE, &ip);
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  return client;
}
/*
File startSDfile(int chipSelect, String filePath) {
  if (!SD.begin(chipSelect)) {
    
  }
  File newFile = SD.open(filePath, FILE_WRITE);
  return newFile;
}
*/
void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
//       setTime(pctime); // Sync Arduino clock to the time received on the serial port
     }
  }
}

time_t requestSync()
{
  //Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}


