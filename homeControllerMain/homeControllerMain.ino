#include <MFRC522.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h> 

boolean match = false;
boolean programMode = false;
int successRead;
byte storedCard[4];
byte readCard[4];
byte masterCard[4];

int wireSecondary = 2;
int wireIR = 3;

int ledPin[54];
int buttonPin[54];

int ledPinArrLen = 0;
int butPinArrLen = 0;

char newStatus = '0';
String line = "";
String lightNo = "";

int btstatus[54];
String preLine = "";
String defaultLine = "W";

void setup() {
  Serial.begin(9600);
  Wire.begin();
  ledPinArrLen = EEPROM.read(1);
  butPinArrLen = EEPROM.read(99);
  if(EEPROM.read(0)==0)
  {
    initializeEEPROM();
  }else
  {
    for (int i = 0; i < ledPinArrLen; i++)
    {
      ledPin[i]=EEPROM.read(i+2);
      pinMode(ledPin[i], OUTPUT);
    }
    for (int i = 0; i < butPinArrLen; i++)
    {
      buttonPin[i]=EEPROM.read(100+i);
      pinMode(buttonPin[i], INPUT_PULLUP);
      btstatus[i] = 0;
    }
  }
}

void loop() {
  LoopSerialLight();
  buttonArray();
}

void buttonArray()
{
  String btArr = "W";
  for (int i = 0; i < butPinArrLen; i++)
  {
    btstatus[i] = 0;
    if (digitalRead(buttonPin[i]) == LOW) {
      btstatus[i] = 1;
    }
    if (btstatus[i] == 1)
    {
      btArr = btArr + "" + '1';
    } else if (btstatus[i] == 0)
    {
      btArr = btArr + "" + '0';
    }
    defaultLine = defaultLine + "0";
  }
  defaultLine = defaultLine + "E";
  btArr = btArr + "" + 'E';
  if (preLine != btArr)
  {
    if (btArr != defaultLine)
    {
      for (int i = 0; i < butPinArrLen; i++)
      {
        if (btstatus[i] == 1)
        {
          ledStateChange(i);
          printSerial();
        }
      }
    }
  }
  preLine = btArr;
}

void LoopSerialLight()
{
  while (Serial.available() > 0)
  {
    line = Serial.readString();
    if(line=="read")
    {
      readEEPROM();
    }else if(line=="clear")
    {
      clearEEPROM();
    }else if(line=="init")
    {
      EEPROM.write(0,0);
      EEPROM.write(1,0);
      EEPROM.write(99,0);
      initializeEEPROM();
    }else if(line=="restart")
    {
      resetFunc();
    }else if(line=="temp")
    {
      String out="";
      wirePrint("temp", wireSecondary);
      delay(5);
      Wire.requestFrom(wireSecondary, 5);
      while (Wire.available()>0) {
        char c = Wire.read();
        out+=c;
      }
      Serial.println(out);
    }else if(line=="getIR")
    {
      wirePrint("getIR", wireIR);
      delay(500);
      Wire.requestFrom(wireIR, 64);
      String out="";
      while (Wire.available()>0) {
        char c = Wire.read();
        out+=c;
      }
      out = out.substring(0, out.indexOf(";"));
      Serial.println(out);
    }else if(line.startsWith("ir:")) { 
      Serial.println(line);
      wirePrint(line, wireIR);      
    } else if(line=="XopenGateY0Z" || line=="XopenGateY1Z")
    {
      String out="";
      wirePrint("gate", wireSecondary);
      delay(2000);
      Wire.requestFrom(wireSecondary, 1);
      while (Wire.available()>0) {
        char c = Wire.read();
        out+=c;
      }
      if(out=="1") { Serial.println("Gate Opened"); }
      else if(out=="0") { Serial.println("Gate Closed"); }
    }else if(line=="XopenGarageY0Z" || line=="XopenGarageY1Z")
    {
      String out="";
      wirePrint("garage", wireSecondary);
      delay(1000);
      Wire.requestFrom(wireSecondary, 1);
      while (Wire.available()>0) {
        char c = Wire.read();
        out+=c;
      }
      if(out=="1") { Serial.println("Garage Door Opened"); }
      else if(out=="0") { Serial.println("Garage Door Closed"); }
    }else if(line=="XopenClothLineY0Z" || line=="XopenClothLineY1Z")
    {
      String out="";
      wirePrint("cloth", wireSecondary);
      delay(1000);
      Wire.requestFrom(wireSecondary, 1);
      while (Wire.available()>0) {
        char c = Wire.read();
        out+=c;
      }
      if(out=="1") { Serial.println("Cloth Line Opened"); }
      else if(out=="0") { Serial.println("Cloth Line Closed"); }
    } else if(line=="XfanY0Z" || line=="XfanY1Z")
    {
      String out="";
      wirePrint("fan", wireSecondary);
      delay(1000);
      Wire.requestFrom(wireSecondary, 1);
      while (Wire.available()>0) {
        char c = Wire.read();
        out+=c;
      }
      if(out=="1") { Serial.println("Cloth Line Opened"); }
      else if(out=="0") { Serial.println("Cloth Line Closed"); }
    } 
    else
    {
      for (int i = 0; i < line.length(); i++)
      {
        if (line.charAt(i) == 'X')
        {
          for (int j = i; j < line.length(); j++)
          {
            if (line.charAt(j) == 'Y')
            {
              if (line.charAt(j + 2) == 'Z')
              {
                if ((line.charAt(j + 1) == '1') || (line.charAt(j + 1) ==  '0'))
                {
                  newStatus = line.charAt(j + 1);
                  lightNo = "";
                  for (int k = i + 1; k < j; k++)
                  {
                    lightNo = lightNo + "" + line.charAt(k);
                  }
                  if (lightNo.toInt() >= 0 && lightNo.toInt() < ledPinArrLen)
                  {
                    if (newStatus == '1')
                    {
                      digitalWrite(ledPin[lightNo.toInt()], HIGH);
                    } else if (newStatus == '0')
                    {
                      digitalWrite(ledPin[lightNo.toInt()], LOW);
                    }
                    i = j + 2;
                  }
                }
              }
              break;
            }
          }
        }
      }
    printSerial();
    }
  }
}

void printSerial()
{
  String serialLine = "R";
  for (int i = 0; i < ledPinArrLen; i++)
  {
    serialLine = serialLine + "" + digitalRead(ledPin[i]);
  }
  serialLine = serialLine + "N";
  Serial.println(serialLine);
}

void wirePrint(String line, int connection)
{
  Wire.beginTransmission(connection);
  char bufferArr[64];
  line.toCharArray(bufferArr, 64);
  Wire.write(bufferArr);
  Wire.endTransmission();
}

void ledStateChange(int val)
{
  if (digitalRead(ledPin[val]) == HIGH)
  {
    digitalWrite(ledPin[val], LOW);
  } else if (digitalRead(ledPin[val]) == LOW)
  {
    digitalWrite(ledPin[val], HIGH);
  }
}

void initializeEEPROM()
{
  Serial.println("Initializing...");
  if(EEPROM.read(1)==0)
  {
    err2:
    Serial.println("Enter no of light pins : ");
    while(Serial.available()==0);
    int val = Serial.readString().toInt();
    if(val<54 && val>0)
    {
      EEPROM.write(1, val);
      for(int i=2;i<EEPROM.read(1)+2;i++)
      {
        err1:
        Serial.print("Set pin number ");
        Serial.print(i-1);
        Serial.println(" : ");
        while(Serial.available()==0);
        int val = Serial.readString().toInt();
        if(val<54 && val>0)
        {
          EEPROM.write(i, val);
        }else
        {
          Serial.println("Error, Invalid pin no");
          goto err1;
        }
      }
    }else
    {
      Serial.println("Error, Invalid no of Pins");
      goto err2;
    }
  }
  EEPROM.write(0, 1);
  if(EEPROM.read(99)==0)
  {
    err3:
    Serial.println("Enter no of button pins : ");
    while(Serial.available()==0);
    int val = Serial.readString().toInt();
    if(val<54 && val>0)
    {
      EEPROM.write(99, val);
      for(int i=0;i<EEPROM.read(99);i++)
      {
        err4:
        Serial.print("Set pin number ");
        Serial.print(i+1);
        Serial.println(" : ");
        while(Serial.available()==0);
        int val = Serial.readString().toInt();
        if(val<54 && val>0)
        {
          EEPROM.write(i+100, val);
        }else
        {
          Serial.println("Error, Invalid pin no");
          goto err4;
        }
      }
    }else
    {
      Serial.println("Error, Invalid no of Pins");
      goto err3;
    }
  }
  Serial.println("Initialization Complete");
  delay(2000);
  resetFunc();
}

void clearEEPROM()
{
  for(int i=0; i<200;i++)
  {
    EEPROM.write(i,0);
  }
  Serial.println("Memory Cleared");
}

void readEEPROM()
{
  String ln2 = "L:";
  for(int i=0;i<EEPROM.read(1);i++)
  {
    String ln = "LED Pin ";
    ln.concat(i+1);
    ln.concat(" = ");
    ln.concat(EEPROM.read(i+2));
    ln2.concat(EEPROM.read(i+2));
    if(i!=(EEPROM.read(1)-1))
    {
      ln2.concat(",");
    }
    Serial.println(ln);
  }
  ln2.concat(";B:");
  for(int i=0;i<EEPROM.read(99);i++)
  {
    Serial.print("Button Pin ");
    Serial.print(i+1);
    Serial.print(" = ");
    Serial.println(EEPROM.read(i+100));
    ln2.concat(EEPROM.read(i+100));
    ln2.concat(",");
  }
  ln2.concat(";");
}

void resetFunc()
{
  asm volatile ("  jmp 0");
}
