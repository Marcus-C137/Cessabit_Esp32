#ifndef ALARM_TEMPS
#define ALARM_TEMPS

#include <Arduino.h>
#include "globals.h"
#include "main.h"

void checkForAlarm(){
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
    }
  }
}

#endif