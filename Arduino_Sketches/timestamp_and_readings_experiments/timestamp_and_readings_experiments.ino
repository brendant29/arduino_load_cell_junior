
#include <TimeLib.h>
#include <HX711.h>

#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 
#define scaleCount 4

int pinsDOUT[scaleCount] = {3,5,7,9}; 
//The pins hooked up to the respective cells' DOUT

int pinsSCK[scaleCount] = {2,4,6,8};
//The pins hooked up to the respective cells' SCK

float calibrations[scaleCount] = {-10000, -10000, -10000, -10000};
//The calibration factors for the cells

HX711 *allCells[scaleCount] = {NULL, NULL, NULL, NULL}; 

void setup()  {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  setSyncProvider( requestSync);  //set function to call when sync required
  
  //setting up the cells
  for(int ii=0; ii<scaleCount; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
  }
  delay(200);
}

void loop(){    
  if (Serial.available()) {
    processSyncMessage();
  }
  if (timeStatus()!= timeNotSet) {
    digitalClockDisplay();  
  } else {
    digitalClockDisplay();
    Serial.println("time not set");
  }
  
  if (timeStatus() == timeSet) {
    digitalWrite(13, HIGH); // LED on if synced
  } else {
    digitalWrite(13, LOW);  // LED off if needs refresh
    //Serial.println("Need Sync");
  }
  delay(2000);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(year());
  Serial.print("-");
  Serial.print(month());
  Serial.print("-");
  Serial.print(day()); 
  Serial.print(",");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second()); 
  for(int ii=0; ii<scaleCount; ii++){
    Serial.print(",");
    Serial.print(allCells[ii]->get_units()); //the actual reading
  }
  //Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
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

