#ifndef OVAOM_H
# define OVAOM_H

#include "Arduino.h"
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <OSCMessage.h>
#include <Wire.h>

// Tweak these parameters to ajust sensitivity
#define SAMPLING_FREQ 		30
#define CALIBRATION_VALUE 	300
#define IDLE_TRIG_TIME 		2000
#define ACTIVE_TRIG_TIME 	100

// Object State
#define IDLE 0
#define ACTIVE 1

// Led State
#define OFF 0
#define CONNECTING 1
#define CONNECTED 2
#define LOW_BATTERY 3

// Battery
#define BATTERY_INTERVAL 1000 // 300000=5mins

class Ovaom
{
public:
	int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
	Ovaom();
	Ovaom(int objectUID);
	
	// Network
	void connectWifi();
	void sendOscMessage(char * addr);
	void sendOscMessage(String addr);
	void sendOscMessage(char * addr, int data);
	void sendOscMessage(String addr, int data);
	void sendOscMessage(OSCMessage *msg);
	

	// MPU
	void setupMPU(const int MPU_addr);
	void getMpuValues();
	int	 checkObjectState();
	void sendObjectState();

	// Preset button
	void setupPresetButton(int buttonPin);
	void presetButton();

	// LED
	int  displayMode = OFF;
	void setupLed(const int ledPin);
	void setLedState(const int state);
	void updateLed();

	// Battery 
	int           batteryLevel();	

private:
	const int			_objectUID;
	WiFiUDP 			Udp;
	IPAddress 			outIp;
	const unsigned int 	outPort = 9001;
	const unsigned int 	localPort = 9002;

	// MPU
	int 	MPU_addr;
	int16_t 	prevAcX = 0, prevAcY = 0, prevAcZ = 0;
	
	// Object state
	bool objectState = 0;
	bool prevObjectState = 0;
	bool instantObjectState = 0;
	bool prevInstantState = 0;

	// checkObjectState()
	unsigned long stableStateTime = 0;

	// Preset button
	int     	_buttonPin = 15;
	int 		buttonState = 0;
	int 		prevButtonState = 0;

	// Led update
	int           _ledPin = 16;
	unsigned long _prevLedMillis = 0;
	int           _ledState = LOW;

};

# endif