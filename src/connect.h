#ifndef CONNECT
#define CONNECT
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <HTTPUpdate.h>
#include "globals.h"
#include "main.h"
#include "memory.h"

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337); 
uint8_t logClient = 99;
IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress netmask(255,255,255,0);

String WIFI_ssid, WIFI_pass, UID, email, password;

int numberOfFoundSSID = 0;
bool WiFiConnectFlag = false;

void wifiConnect();
WiFiClientSecure client = WiFiClientSecure();


char webpage[] PROGMEM = R"=====(
    <html>
    <head>
        <script>
            var Socket;
            function init() {
                Socket = new WebSocket('ws://' + window.location.hostname + ':1337/');
                Socket.onmessage = function(event){
                    document.getElementById("rxconsole").value += event.data;
                }
            }
        </script>
    </head>
    <body onload="javascript:init()">
        <textarea id="rxconsole"></textarea>
    </body>
    </html>
)=====";

void onIndexRequest(AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() + "] HTTP GET request of " + request->url());
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", webpage);
  request->send(response);
}

void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length){
  switch (type){
    case WStype_DISCONNECTED:
      Serial.println("client connected");
      logClient = 0;
      break;
    case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(client_num);
      Serial.printf("[%u] Connection from ", client_num);
      Serial.println(ip.toString());
      logClient = client_num;
      Serial.println("log client in web socket: " + String(logClient));
    }
      break;
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void onHandleSettingsUpdate(AsyncWebServerRequest *request)
{
    bool Debug = true;
    String jObjectPretty;
    String bodyRecieved;

    if(Debug){
        log("in onHandleSettingsUpdate");
    }

    DynamicJsonBuffer jBuffer;
    JsonObject& postRequest = jBuffer.createObject();
    int params = request->params();
    if(Debug){log("Number of Params" + String(params));}
    for(int i = 0; i<params; i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
            bodyRecieved = p->value();
        }
        postRequest[p->name()] = p->value();
        if(Debug){
            log("key recieved: " + p->name());
            log("value recieved: " + p->value());
        }
    }
    
    if(postRequest.containsKey("Wifi_SSID") && postRequest.containsKey("Wifi_password")){
      File configFile = SPIFFS.open("/config.json", "w");
      postRequest.printTo(configFile);
      configFile.close();
      postRequest.prettyPrintTo(jObjectPretty);
      if (Debug){
          log(deviceID);
      }
      request->send(200, "application/json", "{\"status\" : \"ok\"}");
      WiFiConnectFlag = true;
      notifyOfNewDevice = true;
      writeNotifyOfNewDeviceToPreferences(notifyOfNewDevice);
      log("successfully retrieved wifi ssid and password"); 
      if (Debug){
        log("wifi ssid and password present"); 
        log(jObjectPretty);
      }
    }
    else{
        request->send(200, "application/json", "{\"fuck\" : \"off\"}");
        log("unsuccesful in retrieved wifi ssid and password"); 
    }
}

void scanWifi(){
    WiFi.mode(WIFI_STA);
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true, true);
    delay(100);
    numberOfFoundSSID = WiFi.scanNetworks();
}

void onGetKnownWifi(AsyncWebServerRequest *request){

    bool Debug = true;
    DynamicJsonBuffer JSONbuffer_res;
    DynamicJsonBuffer JSONbuffer_WiFis;
    JsonObject& response = JSONbuffer_res.createObject();
    JsonArray& WiFis = JSONbuffer_WiFis.createArray();
    for (int i = 0; i < numberOfFoundSSID; i++){
        WiFis.add(WiFi.SSID(i));
    }
    response["WiFis"] = WiFis;
    String json;
    response.prettyPrintTo(json);
    request->send(200, "application/json", json);
    if(Debug){
        log("in onGetKnownWiFi");
        log("response sent");
        log(json);
    }
}

void wifiConnect()
{
  bool WIFI_Debug = true;
  WiFi.softAPdisconnect();
  WiFi.disconnect(true, true);
  delay(3000);

  //check for stored credentials
  if(SPIFFS.exists("/config.json")){
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();

      DynamicJsonBuffer jBuffer;
      JsonObject& jObject = jBuffer.parseObject(buf.get());
      if(jObject.success() && jObject.containsKey("Wifi_SSID"))
      {
        WIFI_ssid = jObject["Wifi_SSID"].as<String>();
        WIFI_pass = jObject["Wifi_password"].as<String>();
        UID = jObject["UID"].as<String>();
        email = jObject["email"].as<String>();
        password = jObject["password"].as<String>();
        if (WIFI_Debug){
          Serial.println("SSID: ");
          Serial.println(WIFI_ssid);
          Serial.println("password: ");
          Serial.println(WIFI_pass);
          Serial.println("UID: ");
          Serial.println(UID);
          Serial.println("email :");
          Serial.println(email);
          Serial.println("password :");
          Serial.println(password);
        }
        WiFi.mode(WIFI_STA);        
        WiFi.begin(WIFI_ssid.c_str(), WIFI_pass.c_str());
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED) 
        {
          delay(500);
          log("connecting to WiFi");
          digitalWrite(2,!digitalRead(2));
          if ((unsigned long)(millis() - startTime) >= 6000){
            break;
          }

        }
      }
    }
  }

  //CHECK CONNECTION
  if (WiFi.status() == WL_CONNECTED){
    digitalWrite(2,HIGH);
    wifiConnected = true;
  } else {
    WiFi.softAP("Cessabit");
    delay(500);
    IPAddress myIP =  WiFi.softAPIP();
    Serial.print("IN AP MODE - IP address: ");
    Serial.println(myIP);
    wifiConnected = false;
    digitalWrite(2,LOW);     
  }
  if (WIFI_Debug){
    Serial.println("");
    WiFi.printDiag(Serial);
  }
}

void updateFW(String downloadURL, unsigned long VNUM){
  log("Updating");
  writeVNumToSpiffs(VNUM);
  t_httpUpdate_return ret = httpUpdate.update(client,downloadURL);
      // if successful, ESP will restart

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        char updateFailedC[100];
        writeVNumToSpiffs(0);
        sprintf("HTTP_UPDATE_FAILD Error (%d): %s\n", updateFailedC, httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        log(updateFailedC);
        break;

      case HTTP_UPDATE_NO_UPDATES:
        log("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        log("HTTP_UPDATE_OK");
        break;
    }
      log("update failed");
}


#endif