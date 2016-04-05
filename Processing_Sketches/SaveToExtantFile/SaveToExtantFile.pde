import processing.serial.*;
import java.util.Date;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.io.FileWriter;
 
public static final short portIndex = 0;  // select the com port, 0 is the first port
public static final String TIME_HEADER = "T"; //header for arduino serial time message 
public static final char TIME_REQUEST = 7;  // ASCII bell character 
public static final char LF = 10;     // ASCII linefeed
public static final char CR = 13;     // ASCII linefeed
final String DIRECTORY = "C:/Users/brendan/Desktop/Arduino_Load_Cell/Processing_Sketches/SaveToExtantFile/sample_data/";
final int SCALE_COUNT = 4;

FileWriter output = null;

Serial myPort;     // Create object from Serial class
 
String fileName;
String filePath;

boolean needHeader = true;

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

    fileName = "thing2.0"; 
    filePath = DIRECTORY + fileName + ".csv";
    print("filename is: "); println(fileName);
    
    if(!(new File(filePath).isFile())) {
      needHeader = true;
    }
    
    try {
      output = new FileWriter(filePath, true); //the true will append the new data
      if(needHeader) {
        output.write("Date,Time");
        for(int ii = 0; ii < SCALE_COUNT; ii++){
          output.write(",loadcell" + (ii + 1));          
        }
        output.write("\n");
        needHeader = false;
      }
      output.write(val + "\n"); //ONLY SAVES MOST RECENT VALUES!!!
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