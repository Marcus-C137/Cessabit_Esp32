#ifndef ALARM_TEMPS
#define ALARM_TEMPS

#include <Arduino.h>
#include "globals.h"
#include "main.h"

bool checkForAlarm(){
  bool debug = false;
  bool inAlarm = false;
  for(int i=0; i<4; i++){
    if (alarmsMonitored[i] == true){
      if (Temperatures[i]>highAlarmsTemps[i]){
        inAlarm = true;
      }
      if(Temperatures[i]<lowAlarmsTemps[i]){
        inAlarm = true;
      }
    }
  }

  if (debug){
    for (int i=0; i<4; i++){
      Serial.println("alarmsMonitored " + String(i) + " : " + String(alarmsMonitored[i]));
    }
    Serial.println("inAlarm : " + String(inAlarm));
  }
  return inAlarm;
}

void makeAlarmMask(){
  bool debug = true;

  for(int i=0; i<4; i++){
    alarmMask[i] = false;
  }

  for(int i=0; i<4; i++){
    if (alarmsMonitored[i] == true){
      if (Temperatures[i]>highAlarmsTemps[i]){
        alarmMask[i]=true;
      }
      if(Temperatures[i]<lowAlarmsTemps[i]){
        alarmMask[i]=true;
      }
    }else{
      alarmMask[i]=false;
    }
  }

  if(debug){
    for (int i=0; i<4; i++){
      log("alarmsMonitored" + String(i) + " = " + String(alarmsMonitored[i]));
      log("alarmMask" + String(i) + " = " + String(alarmMask[i]));
    }
  }
}

#endif