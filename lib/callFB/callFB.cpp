#include <Arduino.h>  
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "callFB.h"
#include "globals.h"


callResults callFB(callParams params, bool Debug){

    struct callResults results;
    WiFiClientSecure client = WiFiClientSecure();
    
    DynamicJsonBuffer JSONbuffer_req;
    JsonObject& root = JSONbuffer_req.createObject();
    root["UID"]=params.UID;
    root["deviceID"] = params.deviceID;
    JsonArray& temperaturesA = root.createNestedArray("Temperatures");
    JsonArray& setTemperaturesA = root.createNestedArray("setTemperatures");
    JsonArray& lowAlarmsA = root.createNestedArray("setTemperatures"); 
    JsonArray& highAlarmsA = root.createNestedArray("setTemperatures");
    JsonArray& alarmsOnA = root.createNestedArray("setTemperatures");
    for(int i=0; i<4; i++){
        temperaturesA.add(params.Temperatures[i]);
        setTemperaturesA.add(params.localSetTemperatures[i]);
        lowAlarmsA.add(params.localLowAlarms[i]);
        highAlarmsA.add(params.localHighAlarms[i]);
        alarmsOnA.add(params.localAlarmsMonitored[i]);
    } 
    root["Temperatures"] = temperaturesA;
    root["localSetTemperatures"] = setTemperaturesA;
    root["localLowAlarms"] = lowAlarmsA;
    root["localHighAlarms"] = highAlarmsA;
    root["localAlarmsMonitored"] = params.localAlarmsMonitored;
    root["changedTempsOnScreen"] = params.changedTempsOnScreen;    


    String JsonRequest;
    root.printTo(JsonRequest);
    if (Debug){
        char JSONmessageBuffer[800];
        root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        log("JSONmessageBuffer: "); log(JSONmessageBuffer);
        log("JsonRequest: "); log(JsonRequest);
        log("Size of JsonRequest: ");log(sizeof(JsonRequest));
    }

    char firestoreapi[] = "us-central1-zagermonitoringfb.cloudfunctions.net";
    if(client.connect(firestoreapi,443)){

        if (Debug){
            log("FB function connected");
        }
        
        String firestoreURL = "/NodeMCU";
        client.println("POST " + firestoreURL + " HTTP/1.1");
        client.print("Host: "); client.println(firestoreapi);
        client.print("Authorization: Bearer "); client.println(params.idToken);
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.print("Content-Length: ");
        client.println(JsonRequest.length());
        client.println();
        client.println(JsonRequest);
        delay(10);
        yield();

        String body;
        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                break;
            }
        }
        // if there are incoming bytes available
        // from the server, read them and print them:
        while (client.available()) {
            char c = client.read();
            body += c;
        }
        log("Free Heap " + String(ESP.getFreeHeap()));
        client.stop();
        DynamicJsonBuffer jsonBuffer(1024 + body.length());
        JsonObject& responseRoot = jsonBuffer.parseObject(body);
        yield();
        if (responseRoot.success()) {
            
            if(responseRoot.containsKey("Error")){
                char error[500];
                responseRoot["error"].prettyPrintTo(error,sizeof(error));
                log("call FB Error :");
                log(error);
                results.success = false;
                return (results);
            }
            else{
                if (Debug){
                    char root_print[500];
                    responseRoot.prettyPrintTo(root_print, sizeof(root_print));
                    log("body root : "); log(root_print);
                }
            }
            
            if(!responseRoot.containsKey("setTemps")){
                log("callFB callback does not contains setTemps");
                results.success = false;
            }else{
                for (int i =0 ; i<=3; i++ ){
                    results.setTemps[i]= responseRoot["setTemps"][i].as<double>();
                    if (Debug){
                        char setTempsString[5];
                        responseRoot["setTemps"][i].prettyPrintTo(setTempsString,sizeof(setTempsString));
                        log("callFB setTemps ith element: ");
                        log(setTempsString);
                    }
                }

            }

            if(!responseRoot.containsKey("lowAlarms")){
                log("callFB callback does not contain lowAlarms");
                results.success = false;
                return(results);
            }else{
                for (int i =0 ; i<=3; i++ ){
                    results.lowAlarms[i]= responseRoot["lowAlarms"][i].as<double>();
                    if (Debug){
                        char lowAlarmsString[5];
                        responseRoot["lowAlarms"][i].prettyPrintTo(lowAlarmsString, sizeof(lowAlarmsString));
                        log("callFB lowAlarms ith element: ");
                        log(lowAlarmsString);
                    }
                }
            }
            
            if(!responseRoot.containsKey("highAlarms")){
                log("callFB callback does not contain highAlarms");
                results.success = false;
                return(results);
            }else{
                for (int i =0 ; i<=3; i++ ){
                    results.highAlarms[i]= responseRoot["highAlarms"][i].as<double>();
                    if (Debug){
                        char highAlarmsString[5];
                        responseRoot["highAlarms"][i].prettyPrintTo(highAlarmsString, sizeof(highAlarmsString));
                        log("callFB highAlarms ith element: ");
                        log(highAlarmsString);
                    }
                }
            }

            if(!responseRoot.containsKey("iChangedTemps")){
                log("callFB callback does not contain iChangedTemps");
                results.success = false;
                return(results);
            }else{
                results.tempsChangedOnline = responseRoot["iChangedTemps"].as<bool>();
                if(Debug){
                    char tempsChangeOnlinec[20];
                    responseRoot["iChangedTemps"].prettyPrintTo(tempsChangeOnlinec, sizeof(tempsChangeOnlinec));
                    log("iChangedTemps ");
                    log(tempsChangeOnlinec);                    
                }
            }

            if(!responseRoot.containsKey("alarmsMonitored")){
                log("callFB callback does not contain alarmsMonitored");
                results.success=false;
                return(results);
            }else{
                for (int i =0 ; i<=3; i++ ){
                    results.alarmsMonitored[i]= responseRoot["alarmsMonitored"][i].as<bool>();
                    if (Debug){
                        char alarmsMonitoredString[5];
                        responseRoot["alarmsMonitored"][i].prettyPrintTo(alarmsMonitoredString, sizeof(alarmsMonitoredString));
                        log("callFB alarmsMonitored ith element: ");
                        log(alarmsMonitoredString);
                    }
                }
            }

            log("callFB Success");
            results.success = true;
            return (results);
            
        } else {
            log("callFB failed to parse JSON");
            results.success = false;
            return(results);
        }
        
    }
    log("callFB connect Failure");
    results.success = false;
    return(results);
}  
    
