#include <Arduino.h>  
#include <ArduinoJson.h>
#include "callFBalarm.h"
#include "globals.h"


bool callFBalarm(alarmCallParams params, bool Debug){

    WiFiClientSecure client = WiFiClientSecure();


    DynamicJsonBuffer JSONbuffer;
    JsonObject& root = JSONbuffer.createObject();
    JsonArray& temperatures = root.createNestedArray("Temperatures");
    temperatures.add(params.Temperatures[0]);
    temperatures.add(params.Temperatures[1]);    
    temperatures.add(params.Temperatures[2]);    
    temperatures.add(params.Temperatures[3]);
    root["Temperatures"] = temperatures;
    
    JsonArray& tempsMask = root.createNestedArray("tempAlertMask");
    tempsMask.add(params.AlarmMask[0]);
    tempsMask.add(params.AlarmMask[1]);    
    tempsMask.add(params.AlarmMask[2]);    
    tempsMask.add(params.AlarmMask[3]);
    root["alarmMask"] = tempsMask;      

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
            log("callFB alarm connected");
        }
        
        String firestoreURL = "/Alarm";
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

        String body;
        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                Serial.println("headers received");
                break;
            }
        }
        // if there are incoming bytes available
        // from the server, read them and print them:
        while (client.available()) {
            char c = client.read();
            body += c;
        }
        if(Debug){
            log("body :");
            log(body);
        }
        client.stop();
        DynamicJsonBuffer jsonBuffer;
        JsonObject& responseRoot = jsonBuffer.parseObject(body);
        if (responseRoot.success()) {
            
            if(responseRoot.containsKey("Error")){
                const char* error = responseRoot["error"];
                log("callFB alarm Error :");
                log(error);
                return false;
            }
            String root_print;
            responseRoot.printTo(root_print);
            
            if (Debug){
                log("body root : "); log(root_print);
            }
            if(responseRoot["AlarmSubmitted"].as<boolean>() == true){
                log("callFB alarm Success");
                return true;
            }else{
                log("callFB alarm - Alarm Sent reponse not true");
                return false;
            }
            
        } else {
            log("callFB alarm failed to parse JSON");
            return false;
        }
        
    }
    log("callFB alarm connect Failure");
    return false;
}  
    