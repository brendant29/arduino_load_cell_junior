#include <Time.h>
#include <TimeLib.h>

/*
 * SerialOutput sketch
 * Print numbers to the serial port
*/


int number = 0;

void setup()
{
  Serial.begin(9600); // send and receive at 9600 baud
  
}

int number = 0;

void loop()
{
  timeStamp += millis();
  Serial.print(timeStamp+" The number is ");
  Serial.println(number);    // print the number

  delay(1000); // delay one second between numbers
  number++; // to the next number
}
