#include <Arduino.h>
#include <ArduinoJson.h>
#include <math.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <SoftwareSerial.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include "globals.h"
#include "callFB.h"
#include "callFBsub.h"
#include "callFBalarm.h"
#include "Login.h"
#include "newDeviceNotif.h"
#include "alarmTemps.h"
#include "connect.h"
#include "memory.h"
#include "NextionLib.h"
#include "main.h"

#define LOG_ONLINE true
#define LOG_SERIAL true
SoftwareSerial softSerial;

struct signInResults _signInResults;
struct callResults _callResults;
struct callParams _callParams;
struct alarmCallParams _alarmCallParams;
struct callSubResults _callSubResults;

bool relogIN = false;
bool checkSub_andVersion = false;
bool subscribed = false;

unsigned long lastCallTime = 0;
unsigned long callFBTime = 10000;
unsigned long arduinoComTime = 0;

void arduinoCom();

void setup() {
  delay(2000);
  Serial.begin(9600); //Debug
  Serial2.begin(9600); //Nextion
  softSerial.begin(9600, 27, 13); //(Rx, Tx)
  pinMode(25, OUTPUT); //safety relay
  pinMode(25, LOW);
  SPIFFS.begin(true);
  loadVersionNumber();
  loadTempsfromSpiffs();
  loadChangedTempsOnScreenBool();
  loadNotifyOfNewDevice();
  scanWifi();
  alarmWarmUpTick.once(300,ISR_alarmON);
  pinMode(2,OUTPUT); digitalWrite(2, LOW); //led pin
  //CHIP ID
  uint64_t chipMAC = ESP.getEfuseMac();
  Serial.printf("%08X\n",(uint32_t)chipMAC);
  String chipID = String((unsigned long)((uint32_t)chipMAC), HEX);
  Serial.println("chip id is : " + chipID);
  deviceID = deviceID + chipID;
  //Connectivity
  wifiConnect();
  server.on("/postWiFiPassword",HTTP_POST, onHandleSettingsUpdate);
  server.on("/log", HTTP_GET, onIndexRequest);
  server.on("/getKnownWiFi", HTTP_GET, onGetKnownWifi);
  server.begin();
  //webSocket.begin();
  //webSocket.onEvent(onWebSocketEvent);
  
}

void loop() {
  webSocket.loop();
  nextion_loop();
  checkForAlarm();

  if ((millis() - arduinoComTime > 3000) && notTouchingScreen){
    arduinoCom();
    arduinoComTime = millis();
  }
  if (WiFiConnectFlag){wifiConnect(); WiFiConnectFlag = false;} 
  if((WiFi.status() == WL_CONNECTED) && notTouchingScreen)
  {
    if ((_signInResults.idToken == "" || relogIN))
    {
      _signInResults = getSignInKey(email, password, false); 
      relogIN = !_signInResults.success;
      checkSub_andVersion = true;
    }
    if (checkSub_andVersion && _signInResults.success)
    {
      _callSubResults = callFBsub(_signInResults.idToken, false);
      checkSub_andVersion = !_callSubResults.success;
      unsigned long latestVersion = _callSubResults.latestVersion;
      String downloadURL = _callSubResults.DownloadURL;
      if (latestVersion != CURRENT_VERSION && false){
        updateFW(downloadURL, latestVersion);
      }
    }
    if(notifyOfNewDevice && _signInResults.success){
      bool notifiedSuccess = newDeviceNotif(UID, deviceID, _signInResults.idToken, true);
      notifyOfNewDevice = !notifiedSuccess;
      writeNotifyOfNewDeviceToPreferences(notifyOfNewDevice);
    }
    //CALL FB
    if (millis() - lastCallTime > callFBTime){
      lastCallTime = millis();
      _callParams.idToken = _signInResults.idToken;
      _callParams.deviceID = deviceID;
      _callParams.changedTempsOnScreen = changedTempsOnScreen;
      changedTempsOnScreen = false;
      writeChangedTempsToSpiffs(changedTempsOnScreen);
      for(int i=0; i<4; i++){
        _callParams.Temperatures[i] = Temperatures[i];
        _callParams.localSetTemperatures[i] = setTemperatures[i];
        _callParams.localHighAlarms[i] = highAlarmsTemps[i];
        _callParams.localLowAlarms[i] = lowAlarmsTemps[i];
        _callParams.localAlarmsMonitored[i] = alarmsMonitored[i];
      }
      _callResults=callFB(_callParams, false);
      relogIN = !_callResults.success;
    }
    if(_callResults.tempsChangedOnline){
      writeTempstoSpiffs(_callResults.setTemps,_callResults.highAlarms,_callResults.lowAlarms, _callResults.alarmsMonitored);
      loadTempsfromSpiffs();
    }
    if((_signInResults.success == true) && (Alarm_ON_Flag == true))
    {
      for(int i=0; i<4; i++){
        _alarmCallParams.AlarmMask[i] = alarmMask[i];
        _alarmCallParams.Temperatures[i] = Temperatures[i];
      }
      _alarmCallParams.idToken = _signInResults.idToken;
      bool alarmSentSuccessfully = callFBalarm(_alarmCallParams, true);
      relogIN = !alarmSentSuccessfully;
      Alarm_ON_Flag = !alarmSentSuccessfully;
      if (alarmSentSuccessfully){
        alarmWarmUpTick.once(600,ISR_alarmON);
      }
      log("Alert sent successfully " + String(alarmSentSuccessfully));
    }
  }
}

void log(String message){
  if (LOG_ONLINE) {
    String online_message = message + "\r\n";
    if (logClient != 99) {
      webSocket.sendTXT(logClient, online_message);
    }
  }
  if (LOG_SERIAL) {
    Serial.println(message);
  }
}

void arduinoCom(){
  bool Debug = false;

  // Write to Arduino
  DynamicJsonBuffer JSONbuffer;
  JsonObject& root = JSONbuffer.createObject();
  JsonArray& setTemperaturesA = root.createNestedArray("setTemperatures");
  for(int i=0; i<4; i++) setTemperaturesA.add(setTemperatures[i]);
  char cMessage[400];
  String Message;
  root.printTo(Message);
  Message.toCharArray(cMessage, sizeof(cMessage));
  if (Debug){
    log("NodeMCU Sending to Arduino: ");
    log(cMessage);
  }
  softSerial.write(cMessage);

  delay(1000);

  //Read from arduino
  String incoming;
  while (softSerial.available() > 0){
    char inC = softSerial.read();
    incoming += inC;
  }
  softSerial.flush();
  if (Debug) log("recieved software serial: " + incoming);
  DynamicJsonBuffer jb;
  JsonObject& obj = jb.parseObject(incoming);
  if (obj.success()) 
  {
    if (obj.containsKey("currentTemps")){  
      for (int i = 0; i<4; i++){
        Temperatures[i]= obj["currentTemps"][i].as<double>();
        if (int(Temperatures[i]) == int(-196.60)) Temperatures[i] = 0;
      }
    }else if (obj.containsKey("triacTemps")){
      for (int i = 0; i<4; i++){
        triacTemperatures[i] = obj["triacTemps"][i].as<double>();
      } 
    }else{
      log("arduino software serial does not contain any keys");
    }
  }else {
    if (Debug){
      log("Failed to parse Json from software serial");
    }
  }

}

void ISR_alarmON(){
  log("turnging alarm on");
  Alarm_ON_Flag = true;
}

void ISR_screenFocusDone(){
  notTouchingScreen = true;
}