#include "Ovaom.h"

Ovaom::Ovaom(int objectUID) : _objectUID(objectUID) {}

void Ovaom::connectWifi() {

	char ssid[] = "ovaom";          
	char pass[] = "Passovaom"; 
	outIp = IPAddress(192,168,4,1);
	

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pass);
	while (WiFi.status() != WL_CONNECTED) {
	  delay(500);
	  Serial.print(".");
    updateLed();
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	Serial.println("Starting UDP");
	Udp.begin(localPort);
	Serial.print("Local port: ");
	Serial.println(localPort);

}

void Ovaom::sendOscMessage(char * addr) {
  OSCMessage msg(addr);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void Ovaom::sendOscMessage(String addr) {
  OSCMessage msg(addr.c_str());
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void Ovaom::sendOscMessage(char * addr, int data) {
  OSCMessage msg(addr);
  msg.add(data);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void Ovaom::sendOscMessage(String addr, int data) {
  OSCMessage msg(addr.c_str());
  msg.add(data);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void Ovaom::sendOscMessage(OSCMessage *msg) {
  Udp.beginPacket(outIp, outPort);
  msg->send(Udp);
  Udp.endPacket();
  msg->empty();
}

void Ovaom::setupMPU(const int addr) {
	MPU_addr = addr;
	Wire.begin();
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x6B);  // PWR_MGMT_1 register
	Wire.write(0);     // set to zero (wakes up the MPU-6050)
	Wire.endTransmission(true);
}

void Ovaom::getMpuValues() {
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
	Wire.endTransmission(false);
	Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
	AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
	AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
	AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
	Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
	GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
	GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
	GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

int Ovaom::checkObjectState() {
  
  instantObjectState = (abs(AcX - prevAcX) > CALIBRATION_VALUE) ? ACTIVE : IDLE ;
  if (instantObjectState != prevInstantState) {
    stableStateTime = millis();
  }
  else if (instantObjectState == prevInstantState) {
    if (instantObjectState == IDLE && millis() - stableStateTime > IDLE_TRIG_TIME) {
      // Serial.println("Object is idle");
      objectState = IDLE;
      stableStateTime = millis();
    }   
    else if (instantObjectState == ACTIVE && millis() - stableStateTime > ACTIVE_TRIG_TIME) {
      // Serial.println("Object is manipulated");
      objectState = ACTIVE;
      stableStateTime = millis();
    }
  }
  prevInstantState = instantObjectState;
  prevAcX = AcX;
  return (objectState);
}

void Ovaom::sendObjectState() {
  if (objectState != prevObjectState) {
    sendOscMessage("/object/" + String(_objectUID) + "/state", objectState);
    prevObjectState = objectState;
  }
}

void Ovaom::setupPresetButton(const int buttonPin) {
	_buttonPin = buttonPin;
  pinMode(buttonPin, INPUT);

}

void Ovaom::presetButton() {
	buttonState = digitalRead(_buttonPin);
  if (buttonState != prevButtonState) {
    if (buttonState) {
      Serial.println(buttonState);
      sendOscMessage("/object/" + String(_objectUID) + "/presetChange");
    }
    prevButtonState = buttonState;
  }
  delay(1); 
}

void Ovaom::setupLed(const int ledPin) {
  _ledPin = ledPin;
  pinMode(ledPin, OUTPUT);
  displayMode = CONNECTING;
  Serial.println("led setup ok");
}

void Ovaom::updateLed() {
  unsigned long currentMillis = millis();
  switch (displayMode)
  {
    case CONNECTING:
      if (currentMillis - _prevLedMillis > 500 ) {
        _prevLedMillis = currentMillis;
        if (_ledState == LOW)
          _ledState = HIGH;
        else
          _ledState = LOW;
      }
      break;
    
    case CONNECTED:
      _ledState = HIGH;
      break;
    
    case OFF:
      _ledState = LOW;
      break;
    
    case LOW_BATTERY:
      if (currentMillis - _prevLedMillis > 50 ) {
        _prevLedMillis = currentMillis;
        if (_ledState == LOW)
          _ledState = HIGH;
        else
          _ledState = LOW;
      }
      break;
    
    default:
      break;
  }
  digitalWrite(_ledPin, _ledState);
}

int Ovaom::batteryLevel() {
 
  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  int level = analogRead(A0);
  Serial.printf("raw analog value=%d \n", level);
  
  // convert battery level to percent
  // level = map(level, 580, 774, 0, 100);
  // level = map(level, 657, 858, 0, 100); // 580+64=667 774+64=858
  // level = map(level, 570, 750, 0, 100); // empirical with lab power
  // level = map(level, 650, 750, 0, 100); //empiral with 3.7V = 0%
    // level = map(level, 660, 873, 0, 100); //empiricalv2
    level = map(level, 690, 780, 0, 100); //empirical v3
  
  Serial.print("Battery level: "); Serial.print(level); Serial.println("%");
  sendOscMessage("/battery", level);
  if (level <= 35)
    return (LOW);
  else
    return (HIGH);
}