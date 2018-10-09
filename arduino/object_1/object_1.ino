#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include "src/Ovaom.h"

// OBJECT_1 = touch sensor
const int OBJECT_UID = 1;

Ovaom ovaom(OBJECT_UID);

// Touch sensing
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;
uint16_t currtouched = 0;


/*****************/
/*     SETUP     */
/*****************/
void setup(){
  Serial.begin(115200);
  ovaom.setupLed(0);
  ovaom.connectWifi();
  ovaom.setupMPU(0x68);
  ovaom.setupPresetButton(16);


  // Setup touch sensing
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    delay(2000);
    while (1);
  }
  Serial.println("MPR121 found!");
  cap.writeRegister(MPR121_DEBOUNCE, 0x07);
  cap.setThreshholds(60, 30);
  
}


unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
/***************/
/*    LOOP     */
/***************/
void loop(){
  currentMillis = millis();
  if (currentMillis - previousMillis > SAMPLING_FREQ) {
    previousMillis = currentMillis;
    ovaom.getMpuValues();
    ovaom.checkObjectState();
    ovaom.sendObjectState();
    ovaom.presetButton();
    touchSensing();
    sendData();
  }
}

uint16_t sensorData[12];
bool    sensorDataChanged = false;

void touchSensing() {
  // Get the currently touched pads
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
      sensorData[i] = 1;
      sensorDataChanged = true;
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
      sensorData[i] = 0;
      sensorDataChanged = true;
    }
  }
  // reset our state
  lasttouched = currtouched;
}

String paramAddress = "/object/" + String(OBJECT_UID) + "/params";
OSCMessage params(paramAddress.c_str());
int data1, data2, prevData1, prevData2;

void sendData() {
  if (sensorDataChanged) {
    for (int i = 2; i < 8; i++) {
      params.add(sensorData[i]);
    }
  ovaom.sendOscMessage(&params);
  sensorDataChanged = false;
  }
  
}
