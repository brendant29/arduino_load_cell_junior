#include <avr/sleep.h>
#include <avr/power.h>

#include <Wire.h>
#include <DS3232RTC.h>
#include <TimeLib.h>

#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>

#include <SD.h>

#include <HX711.h>

//===============Change values as necessary===================
const String stationName = "Test Station";
String serialCompleteName = "DATALOG.txt";
String uploadBufName = "UPLOAD.txt";
String errorBufName = "ERROR.txt";
#define SCALE_COUNT 4  //how many loadcells
#define TIME_BETWEEN_READINGS .5 //time between readings, in seconds
#define TIME_BETWEEN_SAVES 2     //time between saves, in seconds
#define TIME_BETWEEN_UPLOADS 10  //time between uploads, in seconds
#define RTC_ALARM_PIN 18 //Must be one of: 2, 18, 19
#definte WAKEUP_EVERY_SECOND 1   // If 0, then wakes up every minute instead

#define DEBUG 0 //whether or not to do things over the serial port

//The pins hooked up to the respective cells' DOUT
byte pinsDOUT[SCALE_COUNT] = {23,27,31,35}; 

//The pins hooked up to the respective cells' SCK
byte pinsSCK[SCALE_COUNT] = {25,29,33,37};

//The calibration factors for the cells
float calibrations[SCALE_COUNT] = {-10000, -10000, -10000, -10000};

//The number of NULLs should be the same as the number of loadcells.
HX711 *allCells[SCALE_COUNT] = {NULL, NULL, NULL, NULL}; 

//the gain can be 128 or 64 on channel A, or 32 on channel B
const int gain = 128;

//=============================================================

#if DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x) 
  #define DEBUG_PRINTLN(x) 
#endif

#define CHIP_PIN 4
#define TIME_HEADER  "OK::"   // Header tag for time sync message

unsigned long prevRead = 0;
unsigned long prevSave = 0;
unsigned long prevUpload = 0;
float cellReadings[SCALE_COUNT];
int readsSinceSave = 0;


// These are the interrupt and control pins 
//NOTE: with a shield, pins can't be changed
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin! 3 on shield
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5  //5 on shield
#define ADAFRUIT_CC3000_CS    10 //10 on shield
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

//The name and password of the Wifi access point:
#define WLAN_SSID       "GALAXY_S4_4545"  //name of wifi. cannot be longer than 32 characters!
#define WLAN_PASS       "wljo2151"        //password for wifi

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define DHCP_TIMEOUT 5  //time, in seconds, before it gives up checking DHCP
#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to post to!
#define WEBSITE      "winter-runoff.soils.wisc.edu"

uint32_t ip;

bool sdBegun = false;

//=====================================================================
void setup() {
  
  #if DEBUG
    Serial.begin(9600);
    Serial.println("enter any character to begin");
    while(!Serial.available());
    Serial.readString();
  #endif
  if (SD.begin(CHIP_PIN)) sdBegun = true; 
  
  //setting up the loadcells
  for(int ii=0; ii<SCALE_COUNT; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->set_gain(gain);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
    allCells[ii]->power_down(); // start this way to save power
    cellReadings[ii] = 0;
  }

  setTime(RTC.get()); //set arduino internal time from RTC

  //INPUT mode would likely work, but INPUT_PULLUP means it won't go LOW due to random noise.
  pinMode(RTC_ALARM_PIN, INPUT_PULLUP); 
  
  RTC.alarmInterrupt(2, false); //disable alarm 2
  RTC.alarmInterrupt(1, true);   //enable alarm 1
	
	#if WAKEUP_EVERY_SECOND
	  RTC.setAlarm(ALM1_EVERY_SECOND, 0, 0, 0, 0); //set alarm 1 to fire once per second
  #else // every minute
		RTC.setAlarm(ALM1_MATCH_SECONDS, 0, 0, 0, 0); //set alarm 1 to fire once per minute
	#endif
	
  RTC.alarm(1);  //clear both alarms if they've gone off
  RTC.alarm(2);
  
  prevSave = now();
  prevUpload = now();
  DEBUG_PRINTLN(F("Setup complete."));
}

void loop(){
  setTime(RTC.get());
  
  //Read each loadcell
  if (now() - prevRead > TIME_BETWEEN_READINGS) {
    prevRead = now();
    for (int ii=0; ii<SCALE_COUNT; ii++) {
      allCells[ii]->power_up();
      cellReadings[ii] += allCells[ii]->get_units();
      allCells[ii]->power_down();
    }
    readsSinceSave++;
  }
  
  //average the readings and save
  if (now() - prevSave > TIME_BETWEEN_SAVES) {
    prevSave = now();
    String dataString;
    makeDataString(cellReadings, &readsSinceSave, stationName, &dataString);
    /*
     * Data gets stored in two places: A permanent, serially-complete log for reading
     * off the cards, and a temporary buffer awaiting upload.
     */
    saveToSD(dataString, serialCompleteName);
    saveToSD(dataString, uploadBufName);
    DEBUG_PRINTLN(dataString);
  } 

  if (now() - prevUpload > TIME_BETWEEN_UPLOADS) {
    uploadFromFile(uploadBufName, errorBufName);
    prevUpload = now();
    /*Uploads tend to take a while, so for testing we set prevUpload afterwards,
    * so it doesn't start another upload immediately. This means that it will 
    * actually upload every (TIME_BETWEEN_UPLOADS + (time spent on upload)) 
    * seconds. For more accurate timing, set prevUpload *before* uploading.
    */
  }
  delay(100);
  goToSleep();
}

//=============================================
//==================FUNCTIONS==================
//=============================================

//Given seconds since epoch, return the current date (yyyy-mm-dd)
String dateDisplay(time_t t) {
  String date = String(year(t));
  date += "-";
  date += String(month(t));
  date += "-";
  date += String(day(t));
  return date;
}

//Given seconds since epoch, return the current time (hh:mm:ss)
String timeDisplay(time_t t) {
  String timer = stringDigits(hour(t));
  timer += ":";
  timer += stringDigits(minute(t));
  timer += ":";
  timer += stringDigits(second(t));
  return timer;
}

// utility function for digital clock display: prints preceding colon and leading 0
String stringDigits(int digits){
  String strDigits = "";
  if(digits < 10) {
    strDigits += '0';
  }
  strDigits += String(digits);
  return strDigits;
}

//Assembles a datum for recording (yyyy-mm-dd hh:mm:ss,'stationName',lc1,lc2,...,lcn)
String makeDataString(float cellReadings[], int *readsSinceSave, String stationName, String *myString) {
  *myString = "";
  time_t t = now();
  *myString += dateDisplay(t);
  *myString += " ";
  *myString += timeDisplay(t);
  *myString += ",\'"+stationName+"\'"; 
  for (int ii=0; ii<SCALE_COUNT; ii++) {
    *myString += ",";
    *myString += cellReadings[ii] / *readsSinceSave;
    cellReadings[ii] = 0;
  }
  *readsSinceSave = 0;
  return *myString;
}


bool uploadFromFile(String uploadFileName, String errorFileName) {
  DEBUG_PRINTLN(F("attempting upload"));
  Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER); // you can change this clock speed
  
  if (!cc3000.begin()) {
    DEBUG_PRINTLN(F("Couldn't begin()! Check your wiring?"));
    return false;
  }
  
  DEBUG_PRINTLN(F("CC3000 initialized. Connecting to Wifi..."));
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY, 1)) {
    DEBUG_PRINTLN(F("Wifi connection failed!"));
    cc3000.stop();
    return false;
  }
  
  DEBUG_PRINTLN(F("Wifi connected"));
  // Wait for DHCP to set up our TCP/IP connection and DNS
  int DHCPcount = 0;
  while ((!cc3000.checkDHCP()) && (DHCPcount < (10*DHCP_TIMEOUT))) {
    delay(100);
    DHCPcount++;
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
  
  if (!sdBegun) {
    DEBUG_PRINTLN("card not available!");
    client.close();
    cc3000.disconnect();
    delay(1000);
    cc3000.stop();
    return false;
  }
  
  File uploadFile = SD.open(uploadFileName, FILE_READ);
  String dataString;
  
  while (uploadFile.available()) {
    dataString = uploadFile.readStringUntil('\n');
    DEBUG_PRINTLN("uploadFile: read '"+dataString+"' from '"+uploadFileName+"'");
    // For some reason newlines were appearing among the data lines. Kill them.
    dataString.replace("\n", " ");
    dataString.trim();
    
    //connect to the server, if not connected already
    if (!client.connected()) {
      client = connectToServer(&cc3000);
    }
    
    if (!postString(dataString, client)) {
      saveToSD(dataString, errorFileName);
      #if DEBUG
        saveToSD(dataString, "ERRLOG.TXT"); //keep a log of failed uploads
      #endif
    }
    #if DEBUG
      saveToSD(dataString, "UPLOG.TXT"); //keep a log of all attempted uploads
    #endif
  }

  uploadFile.close();
  client.close();
  cc3000.disconnect();

  DEBUG_PRINT("Juggling Files...");
  /*
   * File juggling! I've heard of this!
   * All the contents in uploadFileName should have been either (1) uploaded, or
   * (2) stuck into errorFileName. So uploadFileName is no longer needed.
   */
  SD.remove(uploadFileName); //get rid of uploaded file
  
  // copy contents of error file into new upload file; retry will be first to be uploaded next time
  uploadFile = SD.open(uploadFileName, FILE_WRITE);
  File errorFile = SD.open(errorFileName, FILE_READ);
  char buf[100];
  int numBytes = 0;
  while (errorFile.available()) {
    numBytes = errorFile.readBytes(buf, 99);
    buf[numBytes] = '\0';
    uploadFile.print(buf);
    uploadFile.flush();
  }
  uploadFile.close();
  errorFile.close();
  
  
  SD.remove(errorFileName); //get rid of error file
  
  DEBUG_PRINTLN("Files juggled.");
  
  delay(1000);
  cc3000.stop();
  return true;
}
/*
 * Our particular server has an HTTP endpoint of /time_series_data/upload, and the POST
 * data should contain "csv_line=our,data,line"
 */
bool postString(String data, Adafruit_CC3000_Client client) {
  String PostData = "csv_line=" + data;
  if (client.connected()) {  //if we're not connected to the website, don't bother.
    DEBUG_PRINT(F("Posting..."));

    //can use print instead of fastrprint 
    //if program space is at a premium
    
    client.fastrprintln("POST /time_series_data/upload HTTP/1.1");
    client.fastrprint("Host: ");
    client.fastrprintln(WEBSITE);
    client.fastrprintln("User-Agent: Arduino/1.0");
    client.fastrprintln("Connection: close");
    client.fastrprint("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
    DEBUG_PRINTLN("posted!");
  }
  delay(1000); //time enough to finish upload

  //if we don't get a specific response from the server, return false
  if (!client.available()) {
    return false;
  }
  if (!processSyncMessage(client)) {
    return false;
  }

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

/*
 * When an upload occurs, the server responds with a text line containing a status code (TIME_HEADER)
 * and a timestamp.
 */
bool processSyncMessage(Adafruit_CC3000_Client client) {
  unsigned long servertime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(client.find(TIME_HEADER)) {
    servertime = client.parseInt();
    if( servertime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      //setTime(servertime); // Sync Arduino clock to the time received from server
    }
    return true;
  }
  else return false;
}

bool saveToSD(String myString, String filePath) {
  if (!sdBegun) {
    DEBUG_PRINTLN(F("No card!"));
    return false;
  }
  DEBUG_PRINTLN("saveToSD writing '"+myString+"' to '"+filePath+"'");
  File newFile = SD.open(filePath, FILE_WRITE);
  newFile.println(myString);
  newFile.close();
  return true;
}

/*
 * The scaly pangolins of Madagascar...
 */
void goToSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  RTC.alarm(1); //clear alarm 1
  delay(100);
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(RTC_ALARM_PIN), wake, LOW);
  delay(100);
  sleep_cpu();

  
  sleep_disable(); //probably redundant; feels safer
  delay(100);
}

void wake() {
  detachInterrupt(5);
  sleep_disable();
}

