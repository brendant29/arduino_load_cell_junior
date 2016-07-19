#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>
#include <Fat16.h>
#include <Fat16util.h>
#include <TimeLib.h>
#include <HX711.h>
//===============Change values as necessary===================
const char stationName[] = "Test Station";
const char fileName[] = "DATALOG.txt";
#define SCALE_COUNT 4
#define TIME_BETWEEN_READINGS 50 //time between readings, in milliseconds
#define TIME_BETWEEN_SAVES 10000 //time between saves, in milliseconds

#define DEBUG 1 //whether or not to do things over the serial port

byte pinsDOUT[SCALE_COUNT] = {8,8,8,8}; 
//The pins hooked up to the respective cells' DOUT

byte pinsSCK[SCALE_COUNT] = {9,9,9,9};
//The pins hooked up to the respective cells' SCK

float calibrations[SCALE_COUNT] = {-10000, -10000, -10000,1000};
//The calibration factors for the cells

HX711 *allCells[SCALE_COUNT] = {NULL, NULL, NULL,NULL}; 
//The number of NULLs should be the same as the number of loadcells.

byte gain = 128;
//the gain can be 128 or 64 on channel A, or 32 on channel B

//=============================================================

#if DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x) 
  #define DEBUG_PRINTLN(x) 
#endif

#define CHIP_PIN 4
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

unsigned long prevRead = 0;
unsigned long prevSave = 0;
unsigned long prevUpload = 0;
float cellReadings[SCALE_COUNT];
byte readsSinceSave = 0;


// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

//The name and password of the Wifi acces point:
const char WLAN_SSID[] = "GALAXY_S4_4545";  // cannot be longer than 32 characters!
const char WLAN_PASS[] = "wljo2151";

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to post to!
const char WEBSITE[] = "winter-runoff.soils.wisc.edu";

uint32_t ip;

//=====================================================================

void setup() {
  
  #if DEBUG
    Serial.begin(9600);
    while(!Serial);
  #endif
  
  
  //setting up the cells
  for(byte ii=0; ii<SCALE_COUNT; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->set_gain(gain);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
    cellReadings[ii] = 0;
  }
}

void loop(){    
  
  #if DEBUG
  //If anything has come in over the serial port, assume it's a time sync message.
  if (Serial.available()) {
    processSyncMessage();
  }
  #endif

  //Read each loadcell
  if (millis() - prevRead > TIME_BETWEEN_READINGS) {
    prevRead = millis();
    //DEBUG_PRINT("reading");
    for (byte ii=0; ii<SCALE_COUNT; ii++) {
      DEBUG_PRINT(ii);
      //DEBUG_PRINTLN(freeRam());
      cellReadings[ii] += allCells[ii]->get_units();
    }
    readsSinceSave++;
    DEBUG_PRINTLN(F("done"));
  }

  //average the readings and save
  if (millis() - prevSave > TIME_BETWEEN_SAVES) {
    char dataString[100];
    makeDataString(cellReadings, &readsSinceSave, stationName, dataString);
    saveString(dataString);
    prevSave = millis();
  } 
}

//=============================================
//==================FUNCTIONS==================
//=============================================

//Given seconds since epoch, return the current date
char* dateDisplay(time_t t,char myString[]) {
  numList(year(t), "", myString);
  numList(month(t), "-", myString);
  numList(day(t), "-", myString);
  return myString;
}

//Given seconds since epoch, return the current time
char* timeDisplay(time_t t,char myString[]) {
  numList(hour(t), "", myString);
  numList(minute(t), ":", myString);
  numList(second(t), ":", myString);
  return myString;
}

// utility function for listing numbers, prints leading separator and 0.
char* numList(float digits, const char *separator, char myString[]){
  strcat(myString,separator);
  if(digits < 10 && digits > 0) {
    strcat(myString,"0");
  }
  itoa(digits, myString+strlen(myString), 10);
}

//Assembles a datum for recording
char* makeDataString(float *cellReadings,byte *readsSinceSave,const char stationName[],char myString[]) {
  strcpy(myString,"");
  time_t t = now();
  dateDisplay(t,myString);
  strcat(myString," ");
  timeDisplay(t,myString);
  strcat(myString,",\'");
  strcat(myString,stationName);
  strcat(myString,"\'");
  for (byte ii=0; ii<SCALE_COUNT; ii++) {
    numList((cellReadings[ii] / *readsSinceSave),",",myString);
    cellReadings[ii] = 0;
  }
  *readsSinceSave = 0;
  return myString;
}

//Does what is needed to save a datum
void saveString(char mystring[]) {
  DEBUG_PRINTLN(mystring);
  // if the file is available, write to it:
  saveToSD(mystring,fileName);
  if (uploadString(mystring)) {
    DEBUG_PRINTLN(F("Upload succeded!"));
  }
  else {
    DEBUG_PRINTLN(F("upload failed!"));
  }
}


//  This function is longer than it 
//  probably should be. In order, it:
//  -initializes the cc3000 
//  -connects to wifi
//  -connects to a server
//  -uploads the string
//  -closes all connections
//  -disables the cc3000

bool uploadString(char mystring[]) {
  DEBUG_PRINTLN(F("attempting upload"));
  Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER); // you can change this clock speed
  if (!cc3000.begin()) {
    DEBUG_PRINTLN(F("Couldn't begin()! Check your wiring?"));
    return false;
  }
  DEBUG_PRINTLN(F("CC3000 initialized. Connecting to Wifi..."));
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY, 1)) {
    //DEBUG_PRINTLN(freeRam());
    DEBUG_PRINTLN(F("Wifi connection failed!"));
    cc3000.stop();
    return false;
  }
  DEBUG_PRINTLN(F("Wifi connected"));
  while (!cc3000.checkDHCP()) {
    delay(100); // ToDo: Insert a DHCP timeout!
    DEBUG_PRINTLN(F("waiting for DHCP"));
  }
  DEBUG_PRINTLN(F("connecting to server..."));
  Adafruit_CC3000_Client client = connectToServer(&cc3000);
  if (!client.connected()) {
    DEBUG_PRINTLN(F("Server connection failed!"));
    cc3000.disconnect();
    delay(1000);
    cc3000.stop();
    return false;
  }
  if (!postString(mystring, client)) {
    DEBUG_PRINTLN(F("Post failed!"));
    client.close();
    cc3000.disconnect();
    delay(1000);
    cc3000.stop();
    return false;
  }
  client.close();
  cc3000.disconnect();
  delay(1000);
  cc3000.stop();
  return true;
}

bool postString(char data[],Adafruit_CC3000_Client client) {
  char PostData[strlen(data)+10];
  strcpy(PostData,"csv_line=");
  strcat(PostData,data);
  if (client.connected()) {
    DEBUG_PRINT(F("Posting..."));

    //can use print instead of fastrprint 
    //if program space is at a premium
    
    client.println(F("POST /time_series_data/upload HTTP/1.1"));
    client.print(F("Host: "));
    client.println(WEBSITE);
    client.println(F("User-Agent: Arduino/1.0"));
    client.println(F("Connection: close"));
    client.print(F("Content-Length: "));
    client.println(strlen(PostData));
    client.println();
    client.println(PostData);
    DEBUG_PRINTLN(PostData);
    DEBUG_PRINTLN(F("posted!"));
    //DEBUG_PRINTLN(freeRam());
  }
  delay(1000); //time enough to finish upload

  /*
  //a bit of code to read a response from the server 
  //and echo it to the serial port:
  
  unsigned long lastRead = millis();
  while (client.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
      lastRead = millis();
    }
  }*/
  
  return true;
}

Adafruit_CC3000_Client connectToServer(Adafruit_CC3000 *cc3000) {
  cc3000->getHostByName(WEBSITE, &ip);
  
  #if DEBUG
    cc3000->printIPdotsRev(ip);
  #endif
  
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

bool saveToSD(char myString[], const char filePath[]) {
  SdCard myCard;
  if (!myCard.init(true, CHIP_PIN)) {
    DEBUG_PRINTLN(F("No card!"));
    return false;
  }
  Fat16 newFile;
  Fat16::init(&myCard);
  newFile.open(filePath, O_CREAT | O_RDWR | O_APPEND);
  newFile.println(myString);
  //DEBUG_PRINTLN(freeRam());
  newFile.close();
}

/*int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}*/
