//code modfied from 
//https://docs.arduino.cc/learn/electronics/servo-motors/
//https://docs.arduino.cc/learn/electronics/servo-motors/
//https://forum.arduino.cc/t/gigar1-usb-filewrite/1131062
//https://www.amazon.com/dp/B07FX84FYT?ref=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&ref_=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&social_share=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&starsLeft=1&th=1


#include <Servo.h>
#include <Arduino_USBHostMbed5.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>
#include <Wire.h>
#include <RTClib.h>

// Pin definitions for valve control
int valvePin1 = 2; 
int valvePin2 = 3; 

// Soil moisture threshold values
int targetLow = 580;
int targetHigh = 630;
int soilVal;

// Status variables
int pumpStatus = 0;
int valveStatus = 0;
int err;

// Time variables
int hour;
int minute;
int second;
uint32_t start;
uint32_t end;

// Create RTC object
RTC_DS3231 rtc;
DateTime now;

// USB storage setup
USBHostMSD msd;
mbed::FATFileSystem usb("usb");
mbed::DigitalOut otg(PB_8, 1);

// Servo for pump control
Servo myservo;  

void setup() {
  // Initialize servo and set initial position
  myservo.attach(9);  
  myservo.write(180);

  // Set pin modes
  pinMode(5, OUTPUT);
  pinMode(valvePin1, OUTPUT);
  pinMode(valvePin2, OUTPUT);
  pinMode(PA_15, OUTPUT); // Enable USB-A port
  digitalWrite(PA_15, HIGH);

  // Initialize USB connection
  msd.connect();
  while (!msd.connected()) {
    msd.connect();
    delay(1000);
  }

  // Mount USB filesystem
  err = usb.mount(&msd);

  // Write initial data to USB
  writeUsbStr("Start Here");
  writeUsbSoil(00000);

  // Initialize RTC
  if (!rtc.begin()) {
    rtc.begin();
    while (1); // Halt if RTC isn't found
  }
}

// Function to log soil moisture data to USB
void writeUsbSoil(int soil) {
  FILE *f = fopen("/usb/soil.txt", "a+");
  fflush(stdout);
  err = fprintf(f, "%d\n", soil);
  fflush(stdout);
  err = fclose(f);
}

// Function to log a text message to USB
void writeUsbStr(char* text) {
  FILE *f = fopen("/usb/data.txt", "a+");
  fflush(stdout);
  err = fprintf(f, "%s\n", text);
  fflush(stdout);
  err = fclose(f);
}

// Function to read soil moisture level
int readSoil() {
  soilVal = analogRead(0);
  writeUsbSoil(soilVal);
}

// Function to toggle the water pump using the servo
void togglePump() {
  myservo.write(130);
  delay(500);
  myservo.write(180);
  pumpStatus = !pumpStatus; // Toggle pump status
}

// Function to open the irrigation valve
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

// Function to close the irrigation valve and log elapsed time
void closeValve() {
  digitalWrite(valvePin1, LOW);
  digitalWrite(valvePin2, HIGH);
  delay(800);
  digitalWrite(valvePin1, LOW);
  digitalWrite(valvePin2, LOW);
  digitalWrite(5, LOW);
  valveStatus = 0;
  end = rtc.now().unixtime(); 

  // Calculate elapsed time
  uint32_t elapsed = end - start;
  int day = rtc.now().day();

  // Format data string
  char dest[30];
  char timeArr[4];
  char dayArr[2];
  sprintf(timeArr, "%d", elapsed);
  sprintf(dayArr, "%d", day);
  strcpy(dest, dayArr);
  strcat(dest, ",");
  strcat(dest, timeArr);

  // Write data to USB
  writeUsbStr(dest);
}

// Function to turn on watering system
void waterOn() {
  if (pumpStatus == 0) {
    togglePump();
    delay(3000);
    openValve();
  }
}

// Function to turn off watering system
void waterOff() {
  if (pumpStatus == 1) {
    togglePump();
    delay(3000);
    closeValve();
  }
}

void loop() {
  readSoil();

  // Check soil moisture and control watering system
  if (soilVal > targetHigh) {
    waterOn();
  } else if (soilVal <= targetLow) {
    waterOff();
  }

  delay(2000); // Wait before next reading
}
