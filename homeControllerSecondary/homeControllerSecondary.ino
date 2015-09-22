#include <Wire.h>
#include <Servo.h> 

Servo leftDoor;
Servo rightDoor;

int enableLeftGate = 9;
int enableRightGate = 10;
int leftIN1 = 7;
int leftIN2 = 8;
int rightIN1 = 11;
int rightIN2 = 12;

int val;
int tempPin = 1;
int pirPin = 3;
bool pirState = false;

//PIR Variables
//int calibrationTime = 30;
int calibrationTime = 5;
long unsigned int lowIn;
long unsigned int pause = 5000;
String prevState = "";
String curState = "";
boolean lockLow = true;
boolean takeLowTime;
long unsigned int curTime = 0;

String request="";

bool gateState = false;
bool garageDoor = false;
bool clothLine = false;

int pos=0;

void setup()
{
  Serial.begin(9600);
  
  Wire.begin(2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  leftDoor.attach(5);
  rightDoor.attach(6);
  leftDoor.write(0);
  rightDoor.write(180);

  pinMode(enableLeftGate, OUTPUT);
  pinMode(enableRightGate, OUTPUT);
  pinMode(leftIN1, OUTPUT);
  pinMode(leftIN2, OUTPUT);
  pinMode(rightIN1, OUTPUT);
  pinMode(rightIN2, OUTPUT);

  digitalWrite(enableLeftGate, LOW);
  digitalWrite(enableRightGate, LOW);
  digitalWrite(leftIN1, LOW);
  digitalWrite(leftIN2, LOW);
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, LOW);
  
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);

  for (int i = 0; i < calibrationTime; i++)
  {
    delay(1000);
  }
  Serial.println("initializing complete");
}


void loop()
{
  readSerial();
  readPIRState();
}

void readSerial()
{
  while (Serial.available() > 0)
  {
    String line = Serial.readString();
    if (line == "temp")
    {
      Serial.println(getTemp());
    } else if (line == "motionOn")
    {
      pirState = true;
    } else if (line == "motionOff")
    {
      pirState = false;
    }
  }
}

void readPIRState()
{

  if ((digitalRead(pirPin) == HIGH) && (pirState == true)) {
    curState = "Motion On";
    if (curState != prevState) {
      Serial.println("Motion On");
      prevState = "Motion On";
    }
    if (lockLow) {
      lockLow = false;
      curTime = millis();
      delay(50);
    }
    takeLowTime = true;
  }

  if ((digitalRead(pirPin) == LOW) && (pirState == true)) {
    curState = "Motion Off";
    if (curState != prevState) {
      Serial.println("Motion Off");
      prevState = "Motion Off";
    }
    if (takeLowTime) {
      lowIn = millis();
      takeLowTime = false;
    }
    if (!lockLow && millis() - lowIn > pause) {
      lockLow = true;
      Serial.print("motion duration ");
      Serial.print((millis() - pause - curTime) / 1000);
      Serial.println(" sec");
      delay(50);
    }
  }
}

void receiveEvent(int howMany)
{
  String line="";
  while(0<Wire.available())
  {
    char c = Wire.read();
    line+=c;
  }
  if(line=="garage")
  {
    Serial.println("Opening/Closing Garage Doors");
    openGarage();
    Serial.println("Done");
  }else if (line=="gate")
  {
    Serial.println("Opening/Closing Gate");
    openGate();
    Serial.println("Done");
  }
  request=line;
}

void requestEvent()
{
  char charVal[10];
  if(request=="temp")
  {
    dtostrf(getTemp(), 5, 2, charVal);
    Wire.write(charVal);
  } else if(request=="garage")
  {
    if(garageDoor) {
      Wire.write("1");
    } else {
      Wire.write("0");
    }
  } else if(request=="gate")
  {
    if(gateState) {
      Wire.write("1");
    }else {
      Wire.write("0");
    }
  }
}

float getTemp()
{
  val = analogRead(tempPin);
  float cel = (( val / 1024.0) * 5000) / 10; 
  return cel;
}

void openGarage()
{
  if(!garageDoor) {
    for(pos = 0; pos <= 180; pos += 1)
    {                                
      leftDoor.write(pos);
      rightDoor.write(180-pos);
      delay(15); 
    } 
    garageDoor=true;
  } else {
    for(pos = 0; pos <= 180; pos += 1)
    {                                
      leftDoor.write(180-pos);
      rightDoor.write(pos);
      delay(100); 
    } 
    garageDoor=false;
  }
}

void openGate()
{
  if(!gateState){
    digitalWrite(leftIN1, LOW);
    digitalWrite(leftIN2, HIGH);
    digitalWrite(rightIN1, LOW);
    digitalWrite(rightIN2, HIGH);
    digitalWrite(enableLeftGate, HIGH);
    digitalWrite(enableRightGate, HIGH);
    delay(1000);
    digitalWrite(enableLeftGate, LOW);
    digitalWrite(enableRightGate, LOW);
    digitalWrite(leftIN1, LOW);
    digitalWrite(leftIN2, LOW);
    digitalWrite(rightIN1, LOW);
    digitalWrite(rightIN2, LOW);
    gateState=true;
  }else {
    digitalWrite(leftIN2, LOW);
    digitalWrite(leftIN1, HIGH);
    digitalWrite(rightIN2, LOW);
    digitalWrite(rightIN1, HIGH);
    digitalWrite(enableLeftGate, HIGH);
    digitalWrite(enableRightGate, HIGH);
    delay(1000);
    digitalWrite(enableLeftGate, LOW);
    digitalWrite(enableRightGate, LOW);
    digitalWrite(leftIN1, LOW);
    digitalWrite(leftIN2, LOW);
    digitalWrite(rightIN1, LOW);
    digitalWrite(rightIN2, LOW);
    gateState=false;
  }
}

