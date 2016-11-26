#include <DS3232RTC.h>
#include <avr/sleep.h>
#include <avr/power.h>

void setup() {
  Serial.begin(9600);
  while(!Serial.available());
  pinMode(18, INPUT_PULLUP);
  Serial.println("setup complete");
}

void loop() {
  Serial.println("start of loop");
  goToSleep();
  Serial.println("end of loop");
  delay(2000);
}

void goToSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  delay(100);
  Serial.println("attaching interrupt");
  attachInterrupt(digitalPinToInterrupt(18), wake, LOW);
  delay(1000);
  Serial.println("sleeping");
  delay(100);
  sleep_cpu();

  
  sleep_disable();
  delay(100);
  Serial.println("awake now");
}

void wake() {
  detachInterrupt(5);
  sleep_disable();
  Serial.println("wake triggered");
}

