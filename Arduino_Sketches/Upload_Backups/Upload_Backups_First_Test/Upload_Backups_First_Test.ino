#include <SD.h>

#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>
#include <ccspi.h>

#include <TimeLib.h>


const String stationName = "Test Station";
String serialComplete = "DATALOG.txt";
String uploadBuffer = "UPLOAD.txt";
String errorBuffer = "ERROR.txt";
unsigned long prevSave = 0;
unsigned long prevUpload = 0;
#define TIME_BETWEEN_SAVES 2000 //time between saves, in milliseconds
#define TIME_BETWEEN_UPLOADS 10000
#define DEBUG 1 //whether or not to do things over the serial port

#if DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x) 
  #define DEBUG_PRINTLN(x) 
#endif

#define CHIP_PIN 4
#define TIME_HEADER  "OK::"   // Header tag for serial time sync message

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

//The name and password of the Wifi acces point:
#define WLAN_SSID       "GALAXY_S4_4545"  // cannot be longer than 32 characters!
#define WLAN_PASS       "wljo2151"

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define DHCP_TIMEOUT 5
#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to post to!
#define WEBSITE      "winter-runoff.soils.wisc.edu"

bool sdBegun = false;

void setup() {
  #if DEBUG
    Serial.begin(9600);
    Serial.println("enter any character to begin");
    while(!Serial.available());
    Serial.readString();
    prevSave = millis();
    prevUpload = millis();
  #endif
  if (SD.begin(CHIP_PIN)) sdBegun = true;
}

void loop() {
  
  //read loadcells here
  
  //average the readings and save
  if (millis() - prevSave > TIME_BETWEEN_SAVES) {
    prevSave = millis();
    String dataString;
    makeDataString(stationName, &dataString);
    saveToSD(dataString, serialComplete);
    saveToSD(dataString, uploadBuffer);
    DEBUG_PRINTLN(dataString);
  } 

  if (millis() - prevUpload > TIME_BETWEEN_UPLOADS) {
    uploadFromFile(uploadBuffer, errorBuffer);
    prevUpload = millis();
  }
}

//=============================================
//==================FUNCTIONS==================
//=============================================

//Given seconds since epoch, return the current date
String dateDisplay(time_t t) {
  String date = String(year(t));
  date += "-";
  date += String(month(t));
  date += "-";
  date += String(day(t));
  return date;
}

//Given seconds since epoch, return the current time
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

//Assembles a datum for recording
String makeDataString(String stationName, String *myString) {
  *myString = "";
  /*time_t t = now();
  *myString += dateDisplay(t);
  *myString += " ";
  *myString += timeDisplay(t);
  *myString += ",\'"+stationName+"\'"; // insert real station name here
  for (int ii=0; ii<4; ii++) {
    *myString += ",";
    *myString += "123.45";
  }/*/
  static int iterates = 0;
  *myString = "Datum number ";
  *myString += String(iterates);
  iterates++;/**/
  return *myString;
}

bool postString(String data,Adafruit_CC3000_Client client) {
  String PostData = "csv_line=" + data;
  if (client.connected()) {
    DEBUG_PRINT(F("Posting..."));

    //can use print instead of fastrprint 
    //if program space is at a premium
    
    client.fastrprintln(F("POST /time_series_data/upload HTTP/1.1"));
    client.fastrprint(F("Host: "));
    client.fastrprintln(F(WEBSITE));
    client.fastrprintln(F("User-Agent: Arduino/1.0"));
    client.fastrprintln(F("Connection: close"));
    client.fastrprint(F("Content-Length: "));
    client.println(PostData.length());
    client.println();
    client.println(PostData);
    DEBUG_PRINTLN(F("posted!"));
  }
  delay(1000); //time enough to finish upload
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
  }*//*
  Serial.println(data);
  Serial.println("Does this \'upload\' succeed?");
  while (!Serial.available());
  if (!Serial.parseInt()) {
    return false;
  }*/
  return true;
}

bool uploadFromFile(String uploadName, String errorName) {
  DEBUG_PRINTLN(F("attempting upload"));
  Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER); // you can change this clock speed
  DEBUG_PRINTLN(F("attempting upload"));
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
    DEBUG_PRINTLN("I don't think the card's available.");
    client.close();
    cc3000.disconnect();
    delay(1000);
    cc3000.stop();
    return false;
  }
  
  File uploadFile = SD.open(uploadName, FILE_READ);
  String dataString;
  
  while (uploadFile.available()) {
    dataString = uploadFile.readStringUntil('\n');
    DEBUG_PRINTLN("uploadFile: read '"+dataString+"' from '"+uploadName+"'");
    dataString.replace("\n", " ");
    dataString.trim();
    if (!postString(dataString, client)) {
      saveToSD(dataString, errorName);
      #if DEBUG
        saveToSD(dataString, "ERRLOG.TXT");
      #endif
    }
    #if DEBUG
      saveToSD(dataString, "REUPLOG.TXT");
    #endif
  }

  uploadFile.close();

  DEBUG_PRINT("Juggling Files...");
  
  SD.remove(uploadName);
  
  client.close();
  cc3000.disconnect();
  
  uploadFile = SD.open(uploadName, FILE_WRITE);
  File errorFile = SD.open(errorName, FILE_READ);
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
  SD.remove(errorName);
  DEBUG_PRINTLN("Files juggled.");
  
  delay(1000);
  cc3000.stop();
  return true;
}




Adafruit_CC3000_Client connectToServer(Adafruit_CC3000 *cc3000) {
  uint32_t *ip;
  cc3000->getHostByName(WEBSITE, ip);
  
  #if DEBUG
    cc3000->printIPdotsRev(*ip);
  #endif
  
  Adafruit_CC3000_Client client = cc3000->connectTCP(*ip, 80);
  return client;
}

bool processSyncMessage(Adafruit_CC3000_Client client) {
  unsigned long servertime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(client.find(TIME_HEADER)) {
    servertime = client.parseInt();
    if( servertime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(servertime); // Sync Arduino clock to the time received on the serial port
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

