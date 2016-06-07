#include <Time.h>
#include <TimeLib.h>

#include <Process.h>

Process date;                 // process used to get the date
int lastSecond = -1;          // need an impossible value for comparison

void setup() {
  Bridge.begin();        // initialize Bridge
  SerialUSB.begin(9600);    // initialize serial

  while (!Serial);              // wait for Serial Monitor to open
  SerialUSB.println("Time Check");  // Title of sketch
  fnord();
}

void loop() {
  int t = now();
  if (lastSecond != t) { // if a second has passed
    // print the time:
    Serial.print(year(t));
    Serial.print("-");
    Serial.print(month(t));
    Serial.print("-");
    Serial.print(day(t));
    Serial.print(" ");
    Serial.print(hour(t));
    Serial.print(":");
    Serial.print(minute(t));
    Serial.print(":");
    Serial.print(second(t));
    Serial.println("");
    lastSecond = t;
  }
}

void fnord() {
  if (!date.running()) {
    date.begin("date");
    date.addParameter("+%s");
    date.run();
  }
  String timeString = date.readString();
  setTime(timeString.toInt());
}
