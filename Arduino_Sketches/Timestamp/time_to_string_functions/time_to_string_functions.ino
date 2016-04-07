#include <Time.h>
#include <TimeLib.h>

void setup() {
  Serial.begin(9600);
}

void loop() {
  time_t t = now();
  Serial.println(dateDisplay(t) + timeDisplay(t));
  delay(1000);
}

String dateDisplay(time_t t) {
  String date = String(year(t));
  date += "-";
  date += String(month(t));
  date += "-";
  date += String(day(t));
  return date;
}

String timeDisplay(time_t t) {
  String timeNow = String(hour(t));
  timeNow += stringDigits(minute(t));
  timeNow += stringDigits(second(t));
  return timeNow;
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

