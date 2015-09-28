// IR LED PWM 3

#include <IRremote.h>
#include <Wire.h>

int RECV_PIN = 11;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

String request="";

void setup()
{
  Serial.begin(9600);
  Wire.begin(3);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  irrecv.enableIRIn(); // Start the receiver
  pinMode(BUTTON_PIN, INPUT);
  pinMode(STATUS_PIN, OUTPUT);
}

// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
int toggle = 0; // The RC5/6 toggle state

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        // Mark
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    codeValue = results->value;
    codeLen = results->bits;
  }
}

void sendCode(int repeat) {
  if (codeType == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(codeValue, codeLen);
      Serial.print("Sent NEC ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == RC5 || codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    } 
    else {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}

int lastButtonState;

void loop() {
  // If button pressed, send the code.
  /*int buttonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Released");
    irrecv.enableIRIn(); // Re-enable receiver
  }

  if (buttonState) {
    Serial.println("Pressed, sending");
    digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState == buttonState);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  } else*/ if(Serial.available()>0) {
    while (Serial.available() > 0) {
      String line = Serial.readString();
      if(line.startsWith("ir:")) {
        irEventSend(line);
      } else if(line.startsWith("getIR")) {
        Serial.println(irEventGet());
      }
    }
    irrecv.enableIRIn(); // Re-enable receiver
  }
  else if (irrecv.decode(&results)) {
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results);
    irrecv.resume(); // resume receiver
    digitalWrite(STATUS_PIN, LOW);
  }
 // lastButtonState = buttonState;
}

void irEventSend(String line) {
    int index1 = line.indexOf(":");
    int index2 = line.indexOf(":", index1+1);
    int index3 = line.indexOf(":", index2+1);
    String type = line.substring(index1+1,index2);
    String len = line.substring(index2+1,index3);
    String code = line.substring(index3+1, line.indexOf(";"));
    Serial.println(type+", "+len+", "+code);
    codeType = type.toInt();
    codeLen = len.toInt();
    codeValue = code.toInt();
    if(codeType == NEC) {
      sendCode(0);
      sendCode(1);
    }else {
      sendCode(0);
      delay(50);
      sendCode(1);
      delay(50);
      sendCode(1);
    }
} 

String irEventGet() {
    String rtnStm = "ir:";
    rtnStm+=codeType;
    rtnStm+=":";
    rtnStm+=codeLen;
    rtnStm+=":";
    rtnStm+=codeValue;
    rtnStm+=";";
    return rtnStm;
}

void receiveEvent(int howMany)
{
  String line="";
  while(0<Wire.available())
  {
    char c = Wire.read();
    if(c!=';') {
      line+=c;
    } else { break; }
  }
  request=line;
  if(request.startsWith("ir:")) {
    irEventSend(line);
  }
  irrecv.resume(); // resume receiver
}

void requestEvent()
{
  char charVal[10];
  if(request=="getIR")
  {
    char buffer[64];
    String str = "";
    str = irEventGet();
    str.toCharArray(buffer, 64);
    Serial.println(buffer);
    Wire.write(buffer);
  } 
  irrecv.resume(); // resume receiver
}





