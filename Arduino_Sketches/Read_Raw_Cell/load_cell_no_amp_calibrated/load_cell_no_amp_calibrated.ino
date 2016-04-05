// How often do we do readings?
const int timeBetweenPrintings = 1000; // We want a reading every 5000 ms;
const int readsPerPrint = 1;

//Calibration Factors:
#define zero_factor 512.0
#define scale_factor 1.0

float analogValueAverage = 0;
float sum = 0;
long timex = 0; //
float reads[readsPerPrint];
int readsSoFar = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("loaded");
}

int loop() {
  float analogValue = (analogRead(0)/*-analogRead(1)*/);
  
  // running average - We smooth the readings a little bit
  if(millis() - timex > readsSoFar*timeBetweenPrintings/readsPerPrint && readsSoFar < readsPerPrint){
    reads[readsSoFar++] = analogValue;
  }
  
  // Is it time to print?
  //if(millis() - timex > timeBetweenPrintings){
  if(millis() > timex + timeBetweenPrintings){
    sum = 0;
    for(int ii=0; ii<readsPerPrint; ii++){
      sum += reads[ii];
    }
    analogValueAverage = sum/readsPerPrint;
    float offsetValue = analogValueAverage - zero_factor;
    float correctedValue = offsetValue*scale_factor;
    Serial.println();
    Serial.println(correctedValue);
    //Serial.println(offsetValue);
    //Serial.println(analogValueAverage);
    timex = millis();
    readsSoFar = 0;
  }
}
