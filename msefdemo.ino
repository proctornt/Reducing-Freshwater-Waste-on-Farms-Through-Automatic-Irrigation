//include statments
#include <Servo.h>
#include <Arduino_USBHostMbed5.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>
//valve connections-note no pin 5 used, used for LED represting solenoid open in code
int valvePin1=2;
int valvePin2=3;
int valvePin3=4;
int valvePin4=6;
int WeatherPin=22;
int valve1Pin =32;
int valve2Pin=27;
int PumpPin=37;




 //calculate soil moisture value by initiating a moisture sensor in the air and then in a cup of water and then using math, determine what values are between 50% and 60% and set the corresponding values here. Note that the soil moisture reads lower values when it is in water
int targetLow=580;
int targetHigh=630;
int targetRainVal=60;
int soilVal;
int soilVal2;
int pumpStatus = 0;
//initialize variables for RTC components 
int err;
int hour;
int minute;
int todayRainPCT;
uint32_t valveOneStart;
uint32_t valveTwoStart;
uint32_t valveOneEnd;
uint32_t valveTwoEnd;

// your network SSID 
char ssid[] = "NoahPhone";
//network password
char pass[] = "12345678";   
int keyIndex = 0;            
int valveOneStatus = 0; 
int valveTwoStatus = 0; 

int status = WL_IDLE_STATUS;

//server name
char server[] = "api.weather.gov";    


WiFiSSLClient client;

// Create an RTC object
RTC_DS3231 rtc;
DateTime now;

//connect to USB
USBHostMSD msd;
mbed::FATFileSystem usb("usb");
mbed::DigitalOut otg(PB_8, 1);

Servo myservo;  


void setup() {
  Serial.begin(9600);
  //Serial.println("in begin");
  myservo.attach(9);  
  myservo.write(180);
  pinMode(5, OUTPUT);
  Serial1.begin(9600);  // HM-10 communication (TX1/RX1)

  pinMode(valvePin1,OUTPUT);
  pinMode(valvePin2,OUTPUT);
  pinMode(valvePin3,OUTPUT);
  pinMode(valvePin4,OUTPUT);
  pinMode(WeatherPin, OUTPUT);
pinMode(valve1Pin, OUTPUT);
pinMode(valve2Pin, OUTPUT);
pinMode(PumpPin, OUTPUT);
  pinMode(52, OUTPUT);
  pinMode(PA_15, OUTPUT); //enable the USB-A port
  digitalWrite(PA_15, HIGH);
  //while (!Serial);
    

  err =  usb.mount(&msd);
  mbed::fs_file_t file;
  struct dirent *ent;
  int dirIndex = 0;
  int res = 0;


    if (WiFi.status() == WL_NO_MODULE) {
    //Serial.println("Communication with WiFi module failed!");
    while (true);

  }
while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    //Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    delay(5000);
    
  }
//Serial.println("Connected to WiFi");
  Serial.println(("Initialized"));

  delay(1000); // Wait for a second
//closeValve();
//closeValve2();
}




int readRain(){
  digitalWrite(WeatherPin, HIGH);
  //Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 443)) {
    //Serial.println("connected to server");

    // Make a HTTP request:
    // client.println("GET /gridpoints/BOX/46,74/forecast HTTP/1.0"); // Oxford, MA
    //client.println("GET /gridpoints/OTX/47,122/forecast HTTP/1.0"); // Spokane, WA
    //client.println("GET /gridpoints/ILN/39,82/forecast HTTP/1.0"); // Columbus, OH
    client.println("GET /gridpoints/LOX/34,118/forecast HTTP/1.0"); // LA
    client.println("User-Agent: arduino, proctornt@gmail.com");
    client.println("Host: api.weather.gov");
    client.println("Accept: application/geo+json,*/*");
    client.println("Connection: close");
    client.println();
    delay(1000);
      digitalWrite(WeatherPin, LOW);

  }

  while (client.available()) {
    String line = client.readStringUntil('\r');
     //Serial.println(line);
    if (line.indexOf("{") != -1) {  // Simple check to make sure it’s a valid JSON response
      DynamicJsonDocument doc(1024);
      
      // Parse the JSON from the server response
      DeserializationError error = deserializeJson(doc, line);

      if (error) {
        //Serial.print("Failed to parse JSON: ");
        //Serial.println(error.f_str());
      }
      else {
        // Extract specific data from JSON
          todayRainPCT= doc["properties"]["periods"][0]["probabilityOfPrecipitation"]["value"].as<long>();
      }
    }
  }
  
  if (!client.connected()) {
    //Serial.println();
    //Serial.println("disconnecting from server.");
    client.stop();

  }
}


int readSoil(){
  
  //soilVal = analogRead(0);

 if (Serial1.available()) {
    
    String c = Serial1.readStringUntil('\n');  // Read a single byte as a char
     char letter = c.charAt(0);  // Get the character at index 
    if (letter=='A'){
    String valStr=(c.substring(2,c.length()));
     soilVal=valStr.toInt();
    }

     if (Serial1.available()) {
    String q = Serial1.readStringUntil('\n');  // Read a single byte as a char
     char letter = q.charAt(0);  // Get the character at index 
    if (letter=='B'){
    String valStr=(q.substring(2,q.length()));
     soilVal2=valStr.toInt();
    }
   }
    }

  
}

// int readSoil2(){
//    //soilVal2 = analogRead(1);
//    if (Serial1.available()) {
//     String q = Serial1.readStringUntil('\n');  // Read a single byte as a char
//      char letter = q.charAt(0);  // Get the character at index 
//     if (letter=='B'){
//     String valStr=(q.substring(2,q.length()));
//      soilVal2=valStr.toInt();
//     }
//    }
 

// }
void togglePump() {
  
  if (pumpStatus==1){
    Serial.println("setting pumpStatus = 0");
    pumpStatus=0;
    digitalWrite(PumpPin, LOW);
  }
  else {
    Serial.println("setting pumpStatus = 1");
    pumpStatus=1;
    digitalWrite(PumpPin, HIGH);
  }

}
void openValve(){
  Serial.println("In openValve()");
 digitalWrite(valve1Pin, HIGH);
   valveOneStatus=1;


  //Serial.print("Valve One Open Start: ");
  //Serial.println(valveOneStart); 
}

void openValve2(){
  Serial.println("In openValveTwo()");
 digitalWrite(valve2Pin, HIGH);

  valveTwoStatus=1;
  
//Serial.print("Valve Two Open Start: ");
  //Serial.println(valveTwoStart);  
}

void closeValve(){
  //Serial.println("In closeValve()");
   digitalWrite(valve1Pin, LOW);

  valveOneStatus=0;

}

void closeValve2(){
     digitalWrite(valve2Pin, LOW);

  
  valveTwoStatus=0;

}

void waterOn(){
  readRain();
  if (todayRainPCT<targetRainVal){
   // Serial.println("it wont rain");
    if (pumpStatus==0){
     // Serial.println("WaterOn: Pump Off Turning Pump On");
        togglePump();
        delay(3000);
    }
    if (valveOneStatus == 0)
      openValve();

  }else{

    //Serial.println("raining");
  }
}
  void waterOn2(){
  readRain();
  if (todayRainPCT<targetRainVal){
   // Serial.println("it wont rain");
    if (pumpStatus==0){
      //Serial.println("WaterOnTwo: Pump off, Turning Pump On");
      togglePump();
      delay(3000);
    }
    if (valveTwoStatus == 0)
      openValve2();
    
  }
}

/*
**********************
 Main Loop
**********************
*/
void loop() {
  digitalWrite(52, HIGH);
  //read soil moisture
  readSoil();
  Serial.println(soilVal);
  delay(1000);
  // readSoil2();
    Serial.println(soilVal2);


  if (soilVal>targetHigh){
    waterOn();
  }
  if (soilVal<=targetLow){
    if (valveOneStatus == 1)
      closeValve();
  }
  if (soilVal2>targetHigh){
    waterOn2();
  }
  if (soilVal2<=targetLow){
    if (valveTwoStatus == 1)
      closeValve2();
  }

  if (valveOneStatus == 0 && valveTwoStatus == 0 && pumpStatus == 1){
    // if both valves are closed, turn the pump off
   // Serial.println("All valves closed and pump is on, turn off");
    togglePump();
  }
  // wait two seconds before polling soil and NWS.
  delay(1000);
  }
