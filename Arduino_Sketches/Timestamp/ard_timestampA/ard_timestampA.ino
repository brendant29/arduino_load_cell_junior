/*
 * SerialOutput sketch
 * Print numbers to the serial port
*/
unsigned long timeStamp;
void setup()
{
  Serial.begin(9600); // send and receive at 9600 baud
  while (Serial.available() < 1) {
    delay(200);
  }
  String timeStr = Serial.readString();
  Serial.print("time hack: ");
  Serial.println(timeStr);
  timeStamp = (unsigned long) (timeStr.toInt());
  Serial.println(timeStamp);
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
