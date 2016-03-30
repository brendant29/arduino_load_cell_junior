import processing.serial.*;
import java.util.Date;
import java.util.Calendar;
import java.util.GregorianCalendar;

public static final short portIndex = 0;  // select the com port, 0 is the first port
public static final String TIME_HEADER = "T"; //header for arduino serial time message 
public static final char TIME_REQUEST = 7;  // ASCII bell character 
public static final char LF = 10;     // ASCII linefeed
public static final char CR = 13;     // ASCII linefeed

Serial myPort;     // Create object from Serial class

Table dataTable = new Table(); //table where we will read in and store values. You can name it something more creative!

int numReadings = 10; //keeps track of how many readings you'd like to take before writing the file. 
int readingCounter = 0; //counts each reading to compare to numReadings. 
 
String fileName;

void setup() {  
  size(200, 200); //<>//
  println(Serial.list());
  println(" Connecting to -> " + Serial.list()[portIndex]);
  myPort = new Serial(this,Serial.list()[portIndex], 9600);
  
  println(" Adding columns");
  dataTable.addColumn("Date");
  dataTable.addColumn("Time");
  dataTable.addColumn("LoadCell 1");
  dataTable.addColumn("LoadCell 2");
    
  println(" Added columns");
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
    TableRow newRow = dataTable.addRow(); //add a row for this new reading
    
    //record sensor information. Customize the names so they match your sensor column names. 
    newRow.setString("Date", sensorVals[0]);
    newRow.setString("Time", sensorVals[1]);
    newRow.setFloat("LoadCell 1", float(sensorVals[2]));
    newRow.setFloat("LoadCell 2", float(sensorVals[3]));
    
    readingCounter++; //optional, use if you'd like to write your file every numReadings reading cycles
    
    //saves the table as a csv in the same folder as the sketch every numReadings. 
    if (readingCounter % numReadings ==0)//The % is a modulus, a math operator that signifies remainder after division. The if statement checks if readingCounter is a multiple of numReadings (the remainder of readingCounter/numReadings is 0)
    {
      fileName = sensorVals[0] + "_" + sensorVals[1] + ".csv"; //this filename is of the form date+time
      print("filename is :"); println(fileName);
      saveTable(dataTable, fileName); //Woo! save it to your computer. It is ready for all your spreadsheet dreams. 
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