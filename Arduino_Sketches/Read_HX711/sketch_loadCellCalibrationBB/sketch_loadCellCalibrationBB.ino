#include "HX711.h"

#define DOUT  3
#define CLK  2

HX711 scale(DOUT, CLK);

// How often do we do readings?
 long time = 0; //
 int timeBetweenReadings = 2000; // How often we want a reading (ms)

void calibrate(HX711 *cell){
  
  if(!Serial){
    return;
  } //return if port not open
  
  Serial.println("Ready the cell for taring, then enter any character");

  //wait for input, then tare
   while (Serial.available() == 0)
   cell->tare();

  //clear the serial buffer
   delay(200);
   while (Serial.available() > 0) {
     Serial.read();
   }

  Serial.println("Place a known weight on the cell, then enter its weight");

  //wait for input
  while (Serial.available() == 0)

  //read cell, compare to input weight, calibrate
  cell->set_scale();
  float cell_weight = cell->get_units();
  delay(200);
  float real_weight = Serial.parseFloat();
  float calibrator = cell_weight / real_weight;
  cell->set_scale(calibrator);
}

void setup() {
  Serial.begin(9600);

  calibrate(&scale);
  Serial.println(scale.get_scale());
}

void loop() {
  
   // Is it time to print?
   if(millis() > time + timeBetweenReadings){
    Serial.println(scale.get_units());
    time = millis();
   }
   
}
