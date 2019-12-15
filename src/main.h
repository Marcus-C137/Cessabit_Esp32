#ifndef MAIN
#define MAIN

#include <Arduino.h>
#include <Ticker.h>
#include <Preferences.h>

Preferences preferences;

String deviceID = "Cessabit-";
bool notifyOfNewDevice = false;
bool changedTempsOnScreen = false;
bool Alarm_ON_Flag = false;
bool Alarm = false;
bool notTouchingScreen = true;
bool wifiConnected = false;

double Temperatures[4] = {0.0, 0.0, 0.0, 0.0};
double triacTemperatures[4] = {0.0};
double setTemperatures[4] = {0.0, 0.0, 0.0, 0.0};
double highAlarmsTemps[4] = {0.0, 0.0, 0.0, 0.0};
double lowAlarmsTemps[4] = {0.0, 0.0, 0.0, 0.0};
bool alarmsMonitored[4] = {false, false, false, false};
bool alarmMask[4] = {false, false, false, false};

Ticker alarmWarmUpTick;

void ISR_alarmON();
void ISR_screenFocusDone();

#endif