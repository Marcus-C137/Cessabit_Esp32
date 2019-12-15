#ifndef MEMORY_H
#define MEMORY_H


#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include "main.h"
#include "globals.h"

int CURRENT_VERSION = 0;

void loadTempsfromSpiffs(){
  bool Debug = false;
  if(Debug) log("in load temps from spiffs");
  if(!SPIFFS.exists("/TemperaturesFile.json")){
    if(Debug) log("TemperaturesFile.json does not exist attempting to make");
    DynamicJsonBuffer jBuffer;
    JsonObject& jobject = jBuffer.createObject();
    JsonArray& setTemps = jobject.createNestedArray("setTemps");
    JsonArray& highAlarms = jobject.createNestedArray("highAlarms");
    JsonArray& lowAlarms = jobject.createNestedArray("lowAlarms");
    JsonArray& alarmsJSON = jobject.createNestedArray("alarmsMonitored");

    for (int i=0; i<4; i++){
        setTemps.add(setTemperatures[i]);
        highAlarms.add(highAlarmsTemps[i]);
        lowAlarms.add(lowAlarmsTemps[i]);
        alarmsJSON.add(alarmsMonitored[i]);
    }
    File setTempsFile = SPIFFS.open("/TemperaturesFile.json","w");
    if(Debug) log("TemperaturesFile.json made");
    jobject.printTo(setTempsFile);
    setTempsFile.close();
  }else{
    File setTempsFile2 = SPIFFS.open("/TemperaturesFile.json","r");
    if (setTempsFile2){
      size_t size = setTempsFile2.size();
      std::unique_ptr<char[]> buf(new char[size]);
      setTempsFile2.readBytes(buf.get(), size);
      setTempsFile2.close();
      DynamicJsonBuffer jBuffer;
      JsonObject& jObject = jBuffer.parseObject(buf.get());
      String jobjectprint;
      jObject.prettyPrintTo(jobjectprint);
      if(Debug) log("File loaded: ");
      if(Debug) log(jobjectprint);
      JsonArray& setTemps = jObject["setTemps"];
      JsonArray& highAlarms = jObject["highAlarms"];
      JsonArray& lowAlarms = jObject["lowAlarms"];
      JsonArray& alarmsJSON = jObject["lowAlarms"];
      if (jObject.success()){
        setTemps.copyTo(setTemperatures);
        highAlarms.copyTo(highAlarmsTemps);
        lowAlarms.copyTo(lowAlarmsTemps);
        alarmsJSON.copyTo(alarmsMonitored);
        if(Debug) log("sucess loading");
        for(int i=0; i<4; i++){
          if(Debug) {
            log(String(setTemperatures[i]));
            log(String(highAlarmsTemps[i]));
            log(String(lowAlarmsTemps[i]));
            log(String(alarmsMonitored[i]));
          }
        }
      }
    }
  }
}

void writeTempstoSpiffs(double set[4], double high[4], double low[4], bool alarms[4]){
  bool debug = false;
  if (debug) log("writing Temps to spiffs");
  if(SPIFFS.exists("/TemperaturesFile.json")){
    DynamicJsonBuffer jBuffer;
    JsonObject& jobject = jBuffer.createObject();
    JsonArray& setTemps = jobject.createNestedArray("setTemps");
    JsonArray& highAlarms = jobject.createNestedArray("highAlarms");
    JsonArray& lowAlarms = jobject.createNestedArray("lowAlarms");
    JsonArray& alarmsJSON = jobject.createNestedArray("alarmsMonitored");

    for (int i=0; i<4; i++){
        setTemps.add(set[i]);
        highAlarms.add(high[i]);
        lowAlarms.add(low[i]);
        alarmsJSON.add(alarms[i]);
    }

    File setTempsFile = SPIFFS.open("/TemperaturesFile.json","r+");
    if (setTempsFile){
      jobject.printTo(setTempsFile);
      String jobjectprint;
      jobject.prettyPrintTo(jobjectprint);
      if (debug){
        log("File rewrote: ");
        log(jobjectprint);
      }
    }
    setTempsFile.close();
  }
}

void loadVersionNumber(){
  bool Debug = false;
  if(!SPIFFS.exists("/VersionFile.json")){
    DynamicJsonBuffer jBuffer;
    JsonObject& jobject = jBuffer.createObject();
    jobject["CURRENT_VERSION"] = 0;
    File VersionFile = SPIFFS.open("/VersionFile.json","w");
    jobject.printTo(VersionFile);
    VersionFile.close();
  }else{
    File versionFile2 = SPIFFS.open("/VersionFile.json","r");
    if (versionFile2){
      size_t size = versionFile2.size();
      std::unique_ptr<char[]> buf(new char[size]);
      versionFile2.readBytes(buf.get(), size);
      versionFile2.close();
      DynamicJsonBuffer jBuffer;
      JsonObject& jObject = jBuffer.parseObject(buf.get());
      if (jObject.success()){
        String jPrint;
        jObject.prettyPrintTo(jPrint);
        if (Debug) log("loaded new Version Num");
        if (Debug) log(jPrint);
        CURRENT_VERSION = jObject["CURRENT_VERSION"].as<int>();
     }
    }
  }
}

void writeVNumToSpiffs(unsigned long VNum){
  bool Debug = false;
  if(SPIFFS.exists("/VersionFile.json")){
    DynamicJsonBuffer jBuffer;
    JsonObject& jobject = jBuffer.createObject();
    jobject["CURRENT_VERSION"] = VNum;
    File VNumfile = SPIFFS.open("/VersionFile.json","r+");
    jobject.printTo(VNumfile);
    String jobjectprint;
    jobject.prettyPrintTo(jobjectprint);
    
    if (Debug) log("Wrote Vnum to Spiffs: ");
    if (Debug) log(jobjectprint);
    
    VNumfile.close();
  }
}
void loadChangedTempsOnScreenBool(){
  bool Debug = false;
  if(!SPIFFS.exists("/ChangedTemps.json")){
    DynamicJsonBuffer jBuffer;
    JsonObject& jobject = jBuffer.createObject();
    jobject["changedTemps"] = false;
    File changedTempsFile = SPIFFS.open("/ChangedTemps.json","w");
    jobject.printTo(changedTempsFile);
    changedTempsFile.close();
  }else{
    File changedTempsFile2 = SPIFFS.open("/ChangedTemps.json","r");
    if (changedTempsFile2){
      size_t size = changedTempsFile2.size();
      std::unique_ptr<char[]> buf(new char[size]);
      changedTempsFile2.readBytes(buf.get(), size);
      changedTempsFile2.close();
      DynamicJsonBuffer jBuffer;
      JsonObject& jObject = jBuffer.parseObject(buf.get());
      if (jObject.success()){
        String jPrint;
        jObject.prettyPrintTo(jPrint);
        if(Debug) log("loaded changed Temps bool" + String(jPrint));
        changedTempsOnScreen = jObject["changedTemps"].as<bool>();
        log("changed temps bool val = " + String(changedTempsOnScreen));
     }
    }
  }
}

void writeChangedTempsToSpiffs(bool changedTemps){
  bool Debug = false;
  if(SPIFFS.exists("/ChangedTemps.json")){
    DynamicJsonBuffer jBuffer;
    JsonObject& jobject = jBuffer.createObject();
    jobject["changedTemps"] = changedTemps;
    File changedTempsFile = SPIFFS.open("/ChangedTemps.json","r+");
    jobject.printTo(changedTempsFile);
    String jobjectprint;
    jobject.prettyPrintTo(jobjectprint);
    
    if (Debug){
      log("changedTemps to Spiffs: ");
      log(jobjectprint);
    }
    changedTempsFile.close();
  }
}

void writeNotifyOfNewDeviceToPreferences(bool NotifVal){
  bool Debug = true;
  preferences.begin("notify", false);
  preferences.putBool("notifVal", NotifVal);
  preferences.end();
  if (Debug) log("writing notifyOfNewDevice = " + String(NotifVal) + "to preferences");
}

void loadNotifyOfNewDevice(){
  bool Debug = true;
  preferences.begin("notify", false);
  notifyOfNewDevice = preferences.getBool("notifVal", false);
  preferences.end();
  if (Debug) log("loading notifyOfNewDevice = " + String(notifyOfNewDevice) + "to RAM");
}
#endif