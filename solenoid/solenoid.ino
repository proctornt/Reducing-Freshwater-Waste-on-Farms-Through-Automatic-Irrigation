int valvePin1=2;
int valvePin2=3;
int valvePin3=4;
int valvePin4=6;

void setup() {
pinMode(valvePin1,OUTPUT);
pinMode(valvePin2,OUTPUT);
pinMode(valvePin3,OUTPUT);
pinMode(valvePin4,OUTPUT);


}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(valvePin1, LOW);
digitalWrite(valvePin2, LOW);
digitalWrite(valvePin3, LOW);
digitalWrite(valvePin4, LOW);

digitalWrite(valvePin1, HIGH);
digitalWrite(valvePin2, LOW);
delay(800);
digitalWrite(valvePin1, LOW);
digitalWrite(valvePin2, LOW);
Serial.println("Valve 1 Open");
delay(10000);
digitalWrite(valvePin1, LOW);
digitalWrite(valvePin2, HIGH);
delay(800);
digitalWrite(valvePin1, LOW);
digitalWrite(valvePin2, LOW);
Serial.println("Valve 1 Closed");
delay(10000);
//valve2
digitalWrite(valvePin3, HIGH);
digitalWrite(valvePin4, LOW);
delay(800);
digitalWrite(valvePin3, LOW);
digitalWrite(valvePin4, LOW);
Serial.println("Valve 2 Open");
delay(10000);
digitalWrite(valvePin3, LOW);
digitalWrite(valvePin4, HIGH);
delay(800);
digitalWrite(valvePin3, LOW);
digitalWrite(valvePin4, LOW);
Serial.println("Valve 2 Closed");
delay(10000);



}
