#include <avr/sleep.h>
#include <avr/power.h>

ISR(PCINT2_vect) {
  
}

void setup() {
  Serial.begin(9600);
  //EICRA=((1 << ISC21) | (1 << ISC20)); // set sense bits for rising edge
  //EIMSK=(1 << INT2); // set intrupt #2 enable mask bits
  PCICR=(1 << PCIE2); // set intrupt #2 pin change bits
  PCMSK2=(1 << PCINT16); // set port k/pin 0 change mask bit
  Serial.print("A");
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  Serial.print("B");
  sleep_enable();
  Serial.print("C");
  delay(100);
  sleep_mode();
  sleep_disable();
  Serial.print("D");
}

void loop() {
  // put your main code here, to run repeatedly:

}
