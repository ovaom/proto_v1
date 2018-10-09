#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Wire.h>
#include "src/Ovaom.h"

// OBJECT_0 = grelot a facettes
const int OBJECT_UID = 0;
#define CALIBRATION_ENABLED false

Ovaom ovaom(OBJECT_UID);

/*****************/
/*     SETUP     */
/*****************/
void setup() {
  Serial.begin(115200);
  ovaom.setupLed(16);
  ovaom.updateLed();
  ovaom.connectWifi();
  ovaom.setupMPU(0x68);
  ovaom.setupPresetButton(15);
  ovaom.displayMode = CONNECTED;
}


unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long prevBattMillis = 0;
/***************/
/*    LOOP     */
/***************/
void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis > SAMPLING_FREQ) {
    previousMillis = currentMillis;
    
    ovaom.getMpuValues();
    if (CALIBRATION_ENABLED)
      calibrateFacet();
      
    ovaom.checkObjectState();
    ovaom.sendObjectState();
    ovaom.presetButton();
    findFacet();
    sendData();
  }
  if (currentMillis - prevBattMillis > BATTERY_INTERVAL) {
    prevBattMillis = currentMillis;
    if (ovaom.batteryLevel() == LOW) {
      ovaom.displayMode = LOW_BATTERY;
    } else {
      ovaom.displayMode = CONNECTED;
    }
  }
  ovaom.updateLed();
}


int AcXMax = -100000, AcXMin = 100000;
int AcYMax = -100000, AcYMin = 100000;
int AcZMax = -100000, AcZMin = 100000;
int loops = 1;
int averageAcX, averageAcY, averageAcZ;

void calibrateFacet() {
  Serial.printf("ovaom.AcX=%d ovaom.AcY=%d ovaom.AcZ=%d \n", ovaom.AcX, ovaom.AcY, ovaom.AcZ);
  if (loops <= 1) {
    Serial.println("Calibrating Facet!!");
    averageAcX = ovaom.AcX;
    averageAcY = ovaom.AcY;
    averageAcZ = ovaom.AcZ;
    loops++;
    return;
  }
  averageAcX += ovaom.AcX;
  averageAcY += ovaom.AcY;
  averageAcZ += ovaom.AcZ;
  if (ovaom.AcX > AcXMax)
    AcXMax = ovaom.AcX;
  else if (ovaom.AcX < AcXMin)
    AcXMin = ovaom.AcX;
  
  if (ovaom.AcY > AcYMax)
    AcYMax = ovaom.AcY;
  else if (ovaom.AcY < AcYMin)
    AcYMin = ovaom.AcY;
  
  if (ovaom.AcZ > AcZMax)
    AcZMax = ovaom.AcZ;
  else if (ovaom.AcZ < AcZMin)
    AcZMin = ovaom.AcZ;

  loops++;
  delay(1);

  if (loops > 128) {
    averageAcX = averageAcX / loops;
    averageAcY = averageAcY / loops;
    averageAcZ = averageAcZ / loops;
    loops = 1;
    Serial.println("***** RESULTS *****");
    Serial.printf("| X | \n avg= %d, max= %d, min= %d \n", averageAcX, AcXMax, AcXMin);
    Serial.printf("| Y | \n avg= %d, max= %d, min= %d \n", averageAcY, AcYMax, AcYMin);
    Serial.printf("| Z | \n avg= %d, max= %d, min= %d \n", averageAcZ, AcZMax, AcZMin);
    Serial.println("----- COPY PASTE THIS: -----");
    Serial.printf("(%d < x && x < %d && %d < y && y < %d && %d < z && z < %d)\n", 
      AcXMin, AcXMax, AcYMin, AcYMax, AcZMin, AcZMax);
    while(1);
  }
}

#define FACET_1(x, y, z) (-3492 < x && x < 6668 && -8040 < y && y < 2872 && -17200 < z && z < -14340)
#define FACET_2(x, y, z) (-10980 < x && x < 260 && -5172 < y && y < 8192 && -17184 < z && z < -11328)
#define FACET_3(x, y, z) (1908 < x && x < 12192 && 10084 < y && y < 17896 && -5852 < z && z < 5640)
#define FACET_4(x, y, z) (-32768 < x && x < -11392 && -3488 < y && y < 6472 && -6748 < z && z < 5660)
#define FACET_5(x, y, z) (-10804 < x && x < 4808 && -3448 < y && y < 8556 && 11352 < z && z < 17008)
#define FACET_6(x, y, z) (1796 < x && x < 15504 && -13408 < y && y < 5500 && 2076 < z && z < 19144)
#define FACET_7(x, y, z) (-14608 < x && x < 1220 && -17712 < y && y < -7316 && -9872 < z && z < 8488)


int currentFacet = 0;

void findFacet() {
  
  if FACET_1(ovaom.AcX, ovaom.AcY, ovaom.AcZ) {
    currentFacet = 1;
  }
  else if FACET_2(ovaom.AcX, ovaom.AcY, ovaom.AcZ) {
    currentFacet = 2;
  }
  else if FACET_3(ovaom.AcX, ovaom.AcY, ovaom.AcZ) {
    currentFacet = 3;
  }
  else if FACET_4(ovaom.AcX, ovaom.AcY, ovaom.AcZ) {
    currentFacet = 4;
  }
  else if FACET_5(ovaom.AcX, ovaom.AcY, ovaom.AcZ) {
    currentFacet = 5;
  }
  else if FACET_6(ovaom.AcX, ovaom.AcY, ovaom.AcZ) {
    currentFacet = 6;
  }
  else if FACET_7(ovaom.AcX, ovaom.AcY, ovaom.AcZ) {
    currentFacet = 7;
  }
  // Serial.printf("Facet #%d !\n", currentFacet);
}

OSCMessage params("/object/0/params");
int data1, data2, prevData1, prevData2;

void sendData() {
  
  data1 = currentFacet;
  if (data1 != prevData1) {
    params.add(data1);
    ovaom.sendOscMessage(&params);
    prevData1 = data1;
  }
}
