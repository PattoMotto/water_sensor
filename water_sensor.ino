//For All Sensor ======================================================
  #define SAMPLING 20  //SamplingTime

//For Pressure sensor ======================================================
  #include <Wire.h>
  #include "BMP085.h"
  BMP085 bmp;
//For Thermometer     ======================================================
  #include <math.h>
  #define THERM_PIN   3  // 10ktherm & 10k resistor as divider.  
/*For SD Card     ======================================================
 *The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 */
  #include <SD.h>
  #include <string.h>
  File myFile;
  float rTemp,rAltitude,rPH;
//For HTTP SERVER              ======================================================
#include <SPI.h>
#include <Ethernet.h>
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);
//FOR PH              ======================================================
  #include <SoftwareSerial.h>            
  SoftwareSerial pH =  SoftwareSerial(2, 3);  //setup and rename soft uart.
//                                  RX|TX
  char stamp_data[15];      //reserve 15 bytes
  byte holding;             //define holding
  byte i;                    //define for loop
  int count = 0;

//Initial==============================================================
  float  tempSensor = 0.00;
  float  altitudeSensor = 0.00;
  float  phSensor = 0.00;
  void   printToUSB();
  void   setupSensor();
  void   readPH();
  float  avgTemp();
  float  avgAltitude();
  void   readAllSensor();

void setupSensor(){
    pH.begin(38400);        //open pH comms
    bmp.begin();  
}
void setupHTTPserver(){
  Ethernet.begin(mac, ip);
  server.begin();
}
void initialSDcard(){
  pinMode(10, OUTPUT);
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
}

  void setup() {    
    Serial.begin(9600);
    setupSensor();
    initialSDcard();
    setupHTTPserver();
  }
  void loop() {
    readAllSensor();
    printToUSB();
    writeDataLogger();
    readDataLogger(20);
    HTTPserver();
  }

//Read Sensor ========================================  
 void readAllSensor(){
  tempSensor  = avgTemp();
  altitudeSensor = avgAltitude();
  readPH();
 }
 
  float avgAltitude(){
    unsigned char i;
    float sumAltitude = 0;
    for(i = 0;i < SAMPLING;i++){
      sumAltitude += bmp.readAltitude();
      delay(25);
    }
    return sumAltitude/SAMPLING;
  }
  
 float avgTemp(){
    int i,sum=0;
    for(i = 0;i < SAMPLING;i++){
      sum+=Thermistor(analogRead(THERM_PIN));
      delay(5);
    }
    return (float)sum/SAMPLING;
  }
  
  
 void readPH(){
   pH.print("r");          //tell stamp to take single reading
pH.print((char)13);      //end in <CR>
delay(100);
pH.listen();                    //listen to pH port
while(pH.available() > 1) {    // wait until greater than 3 bytes
holding=pH.available();      //hold pH stamp info
  
  for(i=0; i <= holding;i++){  //assemble stamp data
    stamp_data[i]= pH.read();
  }
}
stamp_data[holding]='\0';  
phSensor=atof(stamp_data);
 } 

// Print ====================================================
  void printToUSB(){
    Serial.print(" [");
    Serial.print(count++);
    Serial.println("] =======================");
    Serial.print("Altitude is: ");
    Serial.println(altitudeSensor);
    Serial.print("Temp is: ");
    Serial.println(tempSensor);
    Serial.print("Temp(from pressure sensor) is: ");
    Serial.println(bmp.readTemperature());
    Serial.print("Pressure is: ");
    Serial.println(bmp.readPressure());
    //readPH();
    Serial.print("stamp_data: ");   //print data type (added line)
    Serial.println(stamp_data);    //print stamp data (added line)
    Serial.print("phSensor: ");  //print data type
    Serial.println(phSensor);    //print ph float
  }
  
  
  
  
  float Thermistor(int RawADC) {
//    float vcc = 4.91;                       // only used for display purposes, if used
//                                            // set to the measured Vcc.
    float pad = 9880;                       // balance/pad resistor value, set this to
                                            // the measured resistance of your pad resistor
//    float thermr = 10000;                   // thermistor nominal resistance
    long Resistance;  
    float Temp;  // Dual-Purpose variable to save space.
    Resistance=((1024 * pad / RawADC) - pad); 
    Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
    Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
    Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
  
    // BEGIN- Remove these lines for the function not to display anything
    //Serial.print("ADC: "); 
    //Serial.print(RawADC); 
    //Serial.print("/1024");                           // Print out RAW ADC Number
    //Serial.print(", vcc: ");
    //Serial.print(vcc,2);
    //Serial.print(", pad: ");
    //Serial.print(pad/1000,3);
    //Serial.print(" Kohms, Volts: "); 
    //Serial.print(((RawADC*vcc)/1024.0),3);   
    //Serial.print(", Resistance: "); 
    //Serial.print(Resistance);
    //Serial.print(" ohms, ");
    // END- Remove these lines for the function not to display anything
  
    // Uncomment this line for the function to return Fahrenheit instead.
    //temp = (Temp * 9.0)/ 5.0 + 32.0;                  // Convert to Fahrenheit
    return Temp;                                      // Return the Temperature
  }

//SD Card    ===============================================================
void writeDataLogger(){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
//    Serial.print("Writing to test.txt...");
    printData();
    // close the file:
    myFile.close();
//    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
void readDataLogger(unsigned int nRound){
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
    if(myFile.size() > nRound*7*3){
      myFile.seek(myFile.size()-(nRound*7*3));
    }
    // read from the file until there's nothing else in it:
    int n=0;
    while (myFile.available()) {
        getData();
        Serial.print(++n);
        Serial.print("\t");
        Serial.print(rTemp);
        Serial.print("\t");
        Serial.print(rAltitude);
        Serial.print("\t");
        Serial.println(rPH);
    }
//    Serial.println("end");
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
void getData(){
//  Serial.print("GetData\t");
  char t[7];
  unsigned char i,j;
  for(j = 0;j < 3;j++){
      myFile.readBytes(t,7);
      if(j==0){
        rTemp=atol(t)/100.0;
      }
      else if(j==1){
        rAltitude=atol(t)/10.0;
        if(atol(t)/10000 == 1){
          rAltitude = ((rAltitude-1000 )* -1);
        }
      }
      else if(j==2){
        rPH=atol(t)/100.0;
      }
  }
}
void printData(){
  char *ret = (char *)malloc(sizeof(char)*6);
  myFile.println(outTempPH(tempSensor, ret));
  myFile.println(outAltitude(altitudeSensor, ret));
  myFile.println(outTempPH(phSensor, ret));
  free(ret);
}
void fillZero(unsigned int val,char *ret){
  strcpy(ret,"00000");
  unsigned char i=4;
  while(val > 0){
    ret[i--] += (val%10);
    val /= 10;
  }
}
char* outTempPH(float val,char *ret){
  fillZero(val*100, ret);
  return ret;
}
char* outAltitude(float val,char *ret){
  fillZero(val*10, ret);
  if(val < 0)
    ret[0]++;
  return ret;
}
void HTTPserver(){
// listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
                    // add a meta refresh tag, so the browser pulls again every 5 seconds:
          client.println("<meta http-equiv=\"refresh\" content=\"5\">");
          client.println("<h1>HELLO WORKLD./h1>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
