#include <Fat16.h>
#include <Fat16Config.h>
#include <Fat16mainpage.h>
#include <Fat16util.h>
#include <SdCard.h>
#include <SdInfo.h>

#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>

#include <TimeLib.h>
#include <HX711.h>

//===============Change values as necessary===================
const String stationName = "Test Station";
#define SCALE_COUNT 4
#define TIME_BETWEEN_READINGS 500 //time between readings, in milliseconds
#define TIME_BETWEEN_SAVES 10000 //time between saves, in milliseconds
#define DEBUG 1

int pinsDOUT[SCALE_COUNT] = {6,A0,A4,9}; 
//The pins hooked up to the respective cells' DOUT

int pinsSCK[SCALE_COUNT] = {7,A1,A2,A3};
//The pins hooked up to the respective cells' SCK

float calibrations[SCALE_COUNT] = {-10000, -10000, -10000, -10000};
//The calibration factors for the cells

int gain = 128;
//the gain can be 128 or 64 on channel A, or 32 on channel B

//=============================================================

#if DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x) //
  #define DEBUG_PRINTLN(x) //
#endif

const int chipPin = 4;
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

HX711 *allCells[SCALE_COUNT] = {NULL, NULL, NULL, NULL}; 

unsigned long prevRead = 0;
unsigned long prevSave = 0;
unsigned long prevUpload = 0;
float cellReadings[SCALE_COUNT] = {0,0,0,0};
int readsSinceSave = 0;

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
/*Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed
*/
/*
#define WLAN_SSID       "UWNet"           // cannot be longer than 32 characters!
#define WLAN_PASS       ""
*/
#define WLAN_SSID       "GALAXY_S4_4545"           // cannot be longer than 32 characters!
#define WLAN_PASS       "wljo2151"

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
  #if DEBUG
    Serial.begin(9600);
    while(!Serial);
  #endif
  //setting up the cells
  for(int ii=0; ii<SCALE_COUNT; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->set_gain(gain);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
  }  
}

void loop(){    
  #if DEBUG
  if (Serial.available()) {
    processSyncMessage();
  }
  #endif

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
    
    saveString(makeDataString(cellReadings, &readsSinceSave, stationName));
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

void saveString(String mystring) {
  DEBUG_PRINTLN(mystring);
  if (uploadString(mystring)) {
    DEBUG_PRINTLN(F("Upload succeded!"));
  }
  else {
    DEBUG_PRINTLN(F("Upload failed!"));
  }
}

bool uploadString(String mystring) {
  DEBUG_PRINTLN(F("attempting upload"));
  Adafruit_CC3000 *cc3000 = new Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER); // you can change this clock speed
  if (!cc3000->begin()) {
    DEBUG_PRINTLN(F("Couldn't begin()! Check your wiring?"));
    delete cc3000;
    return false;
  }
  DEBUG_PRINTLN(F("CC3000 initialized. Connecting to Wifi..."));
  if (!cc3000->connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY, 1)) {
    DEBUG_PRINTLN(F("Wifi connection failed!"));
    cc3000->stop();
    delete cc3000;
    return false;
  }
  DEBUG_PRINTLN(F("Wifi connected"));
  while (!cc3000->checkDHCP()) {
    delay(100); // ToDo: Insert a DHCP timeout!
    DEBUG_PRINTLN(F("waiting for DHCP"));
  }
  DEBUG_PRINTLN(F("connecting to server..."));
  client = connectToServer(cc3000);
  if (!client.connected()) {
    DEBUG_PRINTLN(F("Server connection failed!"));
    cc3000->disconnect();
    delay(1000);
    cc3000->stop();
    delete cc3000;
    return false;
  }
  if (!postString(mystring, client)) {
    DEBUG_PRINTLN(F("Post failed!"));
    client.close();
    cc3000->disconnect();
    delay(1000);
    cc3000->stop();
    delete cc3000;
    return false;
  }
  client.close();
  cc3000->disconnect();
  delay(1000);
  cc3000->stop();
  delete cc3000;
  return true;
}

/*
bool uploadString(String mystring) {
  bool success = 1;
  Serial.println(F("attempting upload"));
  success = cc3000.begin();
  if (!success) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
  }
  else {
    Serial.println(F("CC3000 initialized. Connecting to Wifi..."));
    success = cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY, 1);
    if (!success) {
      Serial.println(F("Wifi connection failed!"));
    }
    else {
      Serial.println(F("Wifi connected"));
      while (!cc3000.checkDHCP()) {
        delay(100); // ToDo: Insert a DHCP timeout!
        Serial.println(F("waiting for DHCP"));
      }
      Serial.println(F("connecting to server..."));
      client = connectToServer();
      success = client.connected();
      if (!success) {
        Serial.println(F("Server connection failed!"));
      }
      else {
        Serial.println(F("Connected. Attempting post..."));
        success = postString(mystring, client);
        if (!success) {
          Serial.println(F("Post failed!"));
        }
        client.close();
      }
      cc3000.disconnect();
    }
    cc3000.stop();
  }
  return success;
}
*/


bool postString(String data,Adafruit_CC3000_Client client) {
  String PostData = "csv_line=" + data;
  if (client.connected()) {
    DEBUG_PRINT(F("Posting..."));
    client.fastrprintln(F("POST /time_series_data/upload HTTP/1.1"));
    client.fastrprint(F("Host: "));
    client.fastrprintln(F(WEBSITE));
    client.fastrprintln(F("User-Agent: Arduino/1.0"));
    client.fastrprintln(F("Connection: close"));
    client.fastrprint(F("Content-Length: "));
    client.println(PostData.length());
    client.println();
    client.println(PostData);
    DEBUG_PRINTLN(PostData);
    DEBUG_PRINTLN(F("posted!"));
  }
  delay(1000);
  /*
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  */
  return true;
}

Adafruit_CC3000_Client connectToServer(Adafruit_CC3000 *cc3000) {
  cc3000->getHostByName(WEBSITE, &ip);
  DEBUG_PRINTLN(ip);
  cc3000->printIPdotsRev(ip);
  
  // Optional: Do a ping test on the website
  
  DEBUG_PRINT(F("\n\rPinging ")); cc3000->printIPdotsRev(ip); DEBUG_PRINT("...");  
  int replies = cc3000->ping(ip, 5);
  DEBUG_PRINT(replies); DEBUG_PRINTLN(F(" replies"));
  Adafruit_CC3000_Client client = cc3000->connectTCP(ip, 80);
  return client;
}

#if DEBUG
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
#endif


