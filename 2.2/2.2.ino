//https://docs.arduino.cc/learn/electronics/servo-motors/
//https://docs.arduino.cc/tutorials/giga-r1-wifi/giga-usb/
//https://forum.arduino.cc/t/gigar1-usb-filewrite/1131062
//https://www.amazon.com/dp/B07FX84FYT?ref=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&ref_=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&social_share=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&starsLeft=1&th=1
//https://github.com/NorthernWidget/DS3231/tree/master/examples
#include <Servo.h>
#include <Arduino_USBHostMbed5.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>
#include <Wire.h>
#include <RTClib.h>

#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// Valve control pin definitions
int valvePin1 = 2; 
int valvePin2 = 3; 

// Moisture and weather thresholds
int targetLow = 580;
int targetHigh = 630;
int targetRainVal = 15; // Minimum rain percentage to skip watering
int soilVal;
int todayRainPCT; 

// Status variables
int pumpStatus = 0;
int valveStatus = 0;
int err;

// Time tracking variables
uint32_t start;
uint32_t end;

// WiFi credentials
char ssid[] = "Proctor";        
char pass[] = "Proctor#2003!";  
int status = WL_IDLE_STATUS;

// Weather API server details
char server[] = "api.weather.gov";    

// WiFi client for making HTTPS requests
WiFiSSLClient client;

// RTC and USB setup
RTC_DS3231 rtc;
USBHostMSD msd;
mbed::FATFileSystem usb("usb");
mbed::DigitalOut otg(PB_8, 1);

// Servo for controlling the pump
Servo myservo;  

void setup() {
  Serial.begin(9600);
  myservo.attach(9);  
  myservo.write(180); // Set initial servo position

  // Configure pin modes
  pinMode(5, OUTPUT);
  pinMode(valvePin1, OUTPUT);
  pinMode(valvePin2, OUTPUT);
  pinMode(PA_15, OUTPUT); // Enable USB-A port
  digitalWrite(PA_15, HIGH);

  // Connect USB storage
  msd.connect();
  while (!msd.connected()) {
    msd.connect();
    delay(1000);
  }
  err = usb.mount(&msd);
  writeUsbStr("Start Here");
  writeUsbSoil(00000);

  // Initialize RTC
  if (!rtc.begin()) {
    while (1); // Halt if RTC isn't found
  }

  // Connect to WiFi
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
}

// Write soil moisture data to USB storage
void writeUsbSoil(int soil) {
  FILE *f = fopen("/usb/soil.txt", "a+");
  err = fprintf(f, "%d\n", soil);
  fclose(f);
}

// Write general data to USB storage
void writeUsbStr(char* text) {
  FILE *f = fopen("/usb/data.txt", "a+");
  err = fprintf(f, "%s\n", text);
  fclose(f);
}

// Fetch weather data from NWS API
int readRain() {
  if (client.connect(server, 443)) { 
    client.println("GET /gridpoints/ILN/39,82/forecast HTTP/1.0"); // Columbus, OH
    client.println("User-Agent: arduino, proctornt@gmail.com");
    client.println("Host: api.weather.gov");
    client.println("Accept: application/geo+json,*/*");
    client.println("Connection: close");
    client.println();
    delay(1000);
  }

  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (line.indexOf("{") != -1) {  // Check for valid JSON response
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, line);
      if (!error) {
        todayRainPCT = doc["properties"]["periods"][0]["probabilityOfPrecipitation"]["value"].as<long>();
      }
    }
  }

  client.stop(); // Close connection
}

// Read soil moisture sensor data
int readSoil() {
  soilVal = analogRead(0);
  writeUsbSoil(soilVal);
}

// Toggle the water pump using the servo
void togglePump() {
  myservo.write(130);
  delay(500);
  myservo.write(180);
  pumpStatus = !pumpStatus;
}

// Open the valve to allow water flow
void openValve() {
  digitalWrite(valvePin1, HIGH);
  digitalWrite(valvePin2, LOW);
  delay(800);
  digitalWrite(valvePin1, LOW);
  digitalWrite(valvePin2, LOW);
  digitalWrite(5, HIGH);
  valveStatus = 1;
  start = rtc.now().unixtime(); 
}

// Close the valve to stop water flow
void closeValve() {
  digitalWrite(valvePin1, LOW);
  digitalWrite(valvePin2, HIGH);
  delay(800);
  digitalWrite(valvePin1, LOW);
  digitalWrite(valvePin2, LOW);
  digitalWrite(5, LOW);
  valveStatus = 0;
  end = rtc.now().unixtime(); 

  uint32_t elapsed = end - start;
  int day = rtc.now().day();
  char dest[30];
  sprintf(dest, "%d,%d", day, elapsed);
  writeUsbStr(dest);
}

// Turn on watering if needed
void waterOn() {
  readRain();
  if (todayRainPCT < targetRainVal && pumpStatus == 0) {
    togglePump();
    delay(3000);
    openValve();
  }
}

// Turn off watering if needed
void waterOff() {
  if (pumpStatus == 1) {
    togglePump();
    delay(3000);
    closeValve();
  }
}

// Main loop
void loop() {
  readRain(); // Fetch rain probability
  readSoil(); // Read soil moisture data

  if (soilVal > targetHigh) {
    waterOn();
  } else if (soilVal <= targetLow) {
    waterOff();
  }

  delay(10000); // Wait 10 seconds before next cycle
}
