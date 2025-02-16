int valvePin1=2;
int valvePin2=3;

 int water=385;
  int dry=780;
  int range=dry-water;//205
  int intervals=int(range/5);//41
  int eightyPct =water+intervals;//246
  int sixtyPct=water+2*(intervals);//287
  int fourtyPct=water+3*(intervals);//428
  int twentyPct=water+4*(intervals);//469
int val;
int pumpStatus;
int valveStatus;
int err;

#include <Servo.h>
#include <Arduino_USBHostMbed5.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>

//USB code modfied from 
//https://docs.arduino.cc/tutorials/giga-r1-wifi/giga-usb/
//and
//https://forum.arduino.cc/t/gigar1-usb-filewrite/1131062
USBHostMSD msd;
mbed::FATFileSystem usb("usb");
mbed::DigitalOut otg(PB_8, 1);

Servo myservo;  

void setup() {
Serial.begin(9600);
myservo.attach(9);  
myservo.write(180);
pinMode(5, OUTPUT);
pinMode(valvePin1,OUTPUT);
pinMode(valvePin2,OUTPUT);
pinMode(PA_15, OUTPUT); //enable the USB-A port
digitalWrite(PA_15, HIGH);///enable the USB-A port
//attempt to connect to RTC
while (!Serial);
  
  msd.connect();

  while (!msd.connected()) {
    msd.connect();
    Serial.println("Trying");
    delay(1000);
  }
//set up USB
err =  usb.mount(&msd);
mbed::fs_file_t file;
struct dirent *ent;
int dirIndex = 0;
int res = 0;

}
//writes an int to the USB
void writeUsbInt(int text){
FILE *f = fopen("/usb/doc7.txt", "a+");
fflush(stdout);
err = fprintf(f, "%d\n", text);
Serial.println(text);
fflush(stdout);
err = fclose(f);

}
//writes a char array to the USB
void writeUsbStr(char* text){
FILE *f = fopen("/usb/doc7.txt", "a+");
fflush(stdout);
err = fprintf(f, "%s\n", text);
Serial.println(text);
fflush(stdout);
err = fclose(f);
}
//reads and writes soil moisture
int readSoil(){
val = analogRead(0); 
writeUsbInt(val); // Pass the string to writeUsb
Serial.println(val);
}
//turns pump on or off depending on current status
void togglePump() {
myservo.write(130);
delay(500);
myservo.write(180);
Serial.println("Pump toggled");
if (pumpStatus==1){
   pumpStatus=0;
}else {
  pumpStatus=1;
}

}
//opens valve
//modified from Nelsons review https://www.amazon.com/dp/B07FX84FYT?ref=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&ref_=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&social_share=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&starsLeft=1&th=1
void openValve(){
digitalWrite(valvePin1, HIGH);
digitalWrite(valvePin2, LOW);
delay(800);
digitalWrite(valvePin1, LOW);
digitalWrite(valvePin2, LOW);
digitalWrite(5,HIGH);
valveStatus=1;
}

//closes valve
//modified from Nelsons review https://www.amazon.com/dp/B07FX84FYT?ref=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&ref_=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&social_share=cm_sw_r_cso_sms_apin_dp_BQ44CVXAX9NKEJCSBAYA&starsLeft=1&th=1
void openValve(){
void closeValve(){
digitalWrite(valvePin1, LOW);
digitalWrite(valvePin2, HIGH);
delay(800);
digitalWrite(valvePin1, LOW);
digitalWrite(valvePin2, LOW);
digitalWrite(5,LOW);
valveStatus=0;
}
//commences watering
void waterOn(){
if (pumpStatus==0){
  togglePump();
  delay(3000);
  openValve();
}
}
//stops watering
void waterOff(){
  if (pumpStatus==1){
    togglePump();
    delay(3000);//do I need to do thus
    closeValve();
    Serial.println("Water off"); 
    
  }
}
//runs through and if value is below target watering starts and if its not watering stops 
void loop() {
readSoil();
Serial.println(val);
Serial.println(pumpStatus);
Serial.println("Complete");
writeUsbStr("Loop");
if (val>sixtyPct && pumpStatus==0){
  waterOn();
}
else if (val<twentyPct && pumpStatus==1){
  waterOff();
}
delay(2000);
}