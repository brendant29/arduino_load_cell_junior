#include <HX711.h>

#define scaleCount 4
//how many load cells are hooked up

int pinsDOUT[scaleCount] = {3,5,7,9}; 
//The pins hooked up to the respective cells' DOUT

int pinsSCK[scaleCount] = {2,4,6,8};
//The pins hooked up to the respective cells' SCK

float calibrations[scaleCount] = {-10000, -10000, -10000, -10000};
//The calibration factors for the cells

// How often do we do readings?
 unsigned long time = 0; //
 int timeBetweenReadings = 2000; // How often we want a reading (ms)

HX711 *allCells[scaleCount] = {NULL, NULL, NULL, NULL}; 

//Automatically calibrate a load cell, given a few inputs.
/*void calibrate(HX711 *cell){
  
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

  //clear the serial buffer
   while (Serial.available() > 0) {
     Serial.read();
   }
}
*/

void setup() {
  Serial.begin(9600);
  Serial.println("yes, it loaded");

  //setting up the cells
  for(int ii=0; ii<scaleCount; ii++){
    allCells[ii] = new HX711(pinsDOUT[ii], pinsSCK[ii]);
    allCells[ii]->tare();
    allCells[ii]->set_scale(calibrations[ii]);
  }
}

void loop() {
  // Is it time to print?
   if(millis() > time + timeBetweenReadings){

    //read each scale
    for(int ii=0; ii<scaleCount; ii++){
      Serial.print("Cell ");
      Serial.print(ii);
      Serial.println(": ");
      Serial.print(allCells[ii]->get_units()); //the actual reading
      Serial.println(" units");
    }
    time = millis();
   }

}
