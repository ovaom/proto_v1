#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_MCP3008.h>
#include "src/Ovaom.h"

// OBJECT_2 = joystick
const int OBJECT_UID = 2;

//MCP3008 pin connections
#define CS_PIN 15
#define CLOCK_PIN 14
#define MOSI_PIN 13
#define MISO_PIN 12
 
Adafruit_MCP3008 adc;

// Analog in
int joystick[2];
int mic[] = {0, 0, 0, 0, 0};
int micIdx = 0;
unsigned int micAvg = 0;

Ovaom ovaom(OBJECT_UID);

/*****************/
/*     SETUP     */
/*****************/
void setup(){
  Serial.begin(115200);
  ovaom.connectWifi();
  ovaom.setupMPU(0x68);
  ovaom.setupPresetButton(16);

  adc.begin(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);    
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
    readAnalog();
    sendData();
  }
}

int tmp, prevTmp = 0;
void readAnalog() {
  joystick[0] = adc.readADC(0);
  joystick[1] = adc.readADC(1);

  mic[micIdx % 5] = adc.readADC(2); 
  for(int i = 0; i < 5; i++)
  {
    micAvg += mic[i];
  }
  micAvg /= 5;
  micIdx++;
  // Serial.printf(" = %d \n", micAvg);
  
  // Serial.printf("X=%d Y=%d MIC=%d \n", joystick[0], joystick[1]);
}

String paramAddress = "/object/" + String(OBJECT_UID) + "/params";
OSCMessage params(paramAddress.c_str());
float data1, data2, data3, prevData1, prevData2, prevData3;

void sendData() {
  data1 = mapfloat((float)joystick[0], 20.0, 900.0, 0.0, 1.0);
  data2 = mapfloat((float)joystick[1], 20.0, 900.0, 0.0, 1.0);
  data3 = mapfloat((float)micAvg, 0.0, 300.0, 0.0, 1.0);
  // Serial.printf("data1=%d \t data2=%d \t data3=%d \n", data1, data2, data3);
  if (abs(data1 - prevData1) > 0.01 || abs(data2 - prevData2) > 0.01 || abs(data3 - prevData3) > 0.01) {
    params.add(data1).add(data2).add(data3);
    ovaom.sendOscMessage(&params);
    prevData1 = data1;
    prevData2 = data2;
    prevData3 = data3;
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}