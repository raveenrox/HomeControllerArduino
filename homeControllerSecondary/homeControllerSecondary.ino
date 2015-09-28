#include <Wire.h>
#include <Servo.h> 

Servo leftDoor;
Servo rightDoor;
Servo lock;

int CLEN = 16;
int fanEnable = 17;

int CLIN1 = 2;
int CLIN2 = 4;

int enableLeftGate = 9;
int enableRightGate = 10;
int leftIN1 = 7;
int leftIN2 = 8;
int rightIN1 = 12;
int rightIN2 = 13;

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

  leftDoor.attach(6);
  rightDoor.attach(5);
  lock.attach(11);
  leftDoor.write(245);
  rightDoor.write(10);
  lock.write(0);

  pinMode(enableLeftGate, OUTPUT);
  pinMode(enableRightGate, OUTPUT);
  pinMode(leftIN1, OUTPUT);
  pinMode(leftIN2, OUTPUT);
  pinMode(rightIN1, OUTPUT);
  pinMode(rightIN2, OUTPUT);
  pinMode(CLIN1, OUTPUT);
  pinMode(CLIN1, OUTPUT);
  pinMode(CLEN, OUTPUT);
  pinMode(fanEnable, OUTPUT);
  
  digitalWrite(enableLeftGate, LOW);
  digitalWrite(enableRightGate, LOW);
  digitalWrite(leftIN1, LOW);
  digitalWrite(leftIN2, LOW);
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, LOW);
  digitalWrite(CLIN1, LOW);
  digitalWrite(CLIN2, LOW);
  digitalWrite(CLEN, LOW);
  digitalWrite(fanEnable, LOW);
  
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
    } else if (line == "fanOn")
    {
      Serial.println("Fan ON");
      digitalWrite(fanEnable, HIGH);
    } else if (line == "fanOff")
    {
      Serial.println("Fan OFF");
      digitalWrite(fanEnable, LOW);
    } else if (line == "gate") 
    {
      Serial.println("Opening/Closing Gate");
      openGate();
      Serial.println("Done");
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
  } else if( line=="cloth")
  {
    Serial.println("Opening/Closing Cloth Line");
    clothLineOp();
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
  } else if(request=="cloth")
  {
    if(clothLine) {
      Wire.write("1"); 
    } else {
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
    leftDoor.write(120);
    rightDoor.write(130);
    garageDoor=true;
  } else {                              
    leftDoor.write(10);
    rightDoor.write(240);
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
    delay(2000);
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
    delay(2000);
    digitalWrite(enableLeftGate, LOW);
    digitalWrite(enableRightGate, LOW);
    digitalWrite(leftIN1, LOW);
    digitalWrite(leftIN2, LOW);
    digitalWrite(rightIN1, LOW);
    digitalWrite(rightIN2, LOW);
    gateState=false;
  }
}

void clothLineOp()
{
  if(clothLine) {
    digitalWrite(CLIN1, LOW);
    digitalWrite(CLIN2, HIGH);
    digitalWrite(CLEN, HIGH);
    delay(1500);
    //digitalWrite(CLIN2, LOW);
    //digitalWrite(CLEN, LOW);
    clothLine = false;
  } else { 
    digitalWrite(CLIN2, LOW);
    digitalWrite(CLIN1, HIGH);
    digitalWrite(CLEN, HIGH);
    delay(1500);
    //digitalWrite(CLIN1, LOW);
    //digitalWrite(CLEN, LOW);
    clothLine = true;
  }
}

