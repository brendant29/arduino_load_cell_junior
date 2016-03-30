import processing.serial.*;
import java.util.Date;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.io.FileWriter;
 
FileWriter output = null;

public static final short portIndex = 0;  // select the com port, 0 is the first port
public static final String TIME_HEADER = "T"; //header for arduino serial time message 
public static final char TIME_REQUEST = 7;  // ASCII bell character 
public static final char LF = 10;     // ASCII linefeed
public static final char CR = 13;     // ASCII linefeed

Serial myPort;     // Create object from Serial class

int numReadings = 10; //keeps track of how many readings you'd like to take before writing the file. 
int readingCounter = 0; //counts each reading to compare to numReadings. 
 
String fileName;

void setup() {  
  size(200, 200); //<>//
  println(Serial.list());
  println(" Connecting to -> " + Serial.list()[portIndex]);
  myPort = new Serial(this,Serial.list()[portIndex], 9600);
  
  delay(2000);
  println(getTimeNow());
  long t = getTimeNow();
  sendTimeMessage(TIME_HEADER, t);   
}

void draw() {
  
}

void serialEvent(Serial myPort){
  String val = myPort.readStringUntil('\n'); //The newline separator separates each Arduino loop. We will parse the data by each newline separator. 
  if (val!= null) { //We have a reading! Record it.
    val = trim(val); //gets rid of any whitespace or Unicode nonbreakable space
    print("Val: ");println(val); //Optional, useful for debugging. If you see this, you know data is being sent. Delete if  you like. 
    String sensorVals[] = split(val, ','); //parses the packet from Arduino and places the valeus into the sensorVals array. I am assuming floats. Change the data type to match the datatype coming from Arduino. 
    //print("SensorVals[]: "); println(sensorVals);
    
    
    
    readingCounter++; //optional, use if you'd like to write your file every numReadings reading cycles
    
    if (readingCounter % numReadings ==0)//The % is a modulus, a math operator that signifies remainder after division. The if statement checks if readingCounter is a multiple of numReadings (the remainder of readingCounter/numReadings is 0)
    {
      fileName = "thing.csv"; //this filename is of the form date+time
      print("filename is :"); println(fileName);
      
      try {
        output = new FileWriter(fileName, true); //the true will append the new data
        output.write(sensorVals + "\n");
        println("saved to file");
      }
      catch (IOException e) {
        println("It Broke");
        e.printStackTrace();
      }
      finally {
        if (output != null) {
          try {
            output.close();
          } catch (IOException e) {
            println("Error while closing the writer");
          }
        }
      }
    }
  }
}
 
void sendTimeMessage(String header, long time) {  
  String timeStr = String.valueOf(time);  
  myPort.write(header);  // send header and time to arduino
  myPort.write(timeStr); 
  myPort.write('\n');  
}

long getTimeNow(){
  // java time is in ms, we want secs    
  Date d = new Date();
  Calendar cal = new GregorianCalendar();
  long current = d.getTime()/1000;
  long timezone = cal.get(cal.ZONE_OFFSET)/1000;
  long daylight = cal.get(cal.DST_OFFSET)/1000;
  return current + timezone + daylight; 
}