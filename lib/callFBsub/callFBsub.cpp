#include <Arduino.h>  
#include <ArduinoJson.h>
#include "callFBsub.h"
#include "globals.h"


callSubResults callFBsub(String idToken, bool Debug){

    struct callSubResults results;
    WiFiClientSecure client = WiFiClientSecure();
    char firestoreapi[] = "us-central1-zagermonitoringfb.cloudfunctions.net";
    
    if(client.connect(firestoreapi,443)){
        
        if (Debug){
            Serial.println("client connected");

            log("callFB alarm connected");
        }
        
        String firestoreURL = "/SubCheck";
        client.println("GET " + firestoreURL + " HTTP/1.1");
        client.print("Host: "); client.println(firestoreapi);
        client.print("Authorization: Bearer "); client.println(idToken);
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        delay(10);

        String body;
        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") {
                Serial.println("headers received");
                break;
            }
        }
        while (client.available()) {
            char c = client.read();
            body += c;
        }
        
        if(Debug){
            log("body :");
            log(body);
        }
        client.stop();
        client.flush();
        delay(10);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& responseRoot = jsonBuffer.parseObject(body);
        if (responseRoot.success()) {
            
            if(responseRoot.containsKey("Error")){
                const char* error = responseRoot["error"];
                log("callFB Error :");
                log(error);
                return(results);
            }else{
                results.success = true;
            }
            String root_print;
            responseRoot.printTo(root_print);
            
            if (Debug){
                log("body root : "); log(root_print);
            }
            if(responseRoot["SubActive"].as<boolean>() == true){
                log("callFB sub Success SubActive = true");
                results.subscribed = true;
                
            }else{
                log("callFB alarm - Alarm Sent reponse not true");
                results.subscribed = false;
            }

            if(!responseRoot.containsKey("newestVersion")){
                log("callFB callback does not contain newestVersion");
                results.latestVersion=0;
                return(results);
            }else{
                results.latestVersion = responseRoot["newestVersion"].as<int>();
                if(Debug){
                    char newestVersionC[50];
                    responseRoot["newestVersion"].prettyPrintTo(newestVersionC, sizeof(newestVersionC));
                    log("newestVersion ");
                    log(newestVersionC);                    
                }
            }
            if(!responseRoot.containsKey("downloadURL")){
                log("callFB callback does not contain downloadURL");
                results.DownloadURL=" ";
                return(results);
            }else{
                results.DownloadURL = responseRoot["downloadURL"].as<String>();
                if(Debug){
                    char downloadURLc[300];
                    responseRoot["downloadURL"].prettyPrintTo(downloadURLc, sizeof(downloadURLc));
                    log("downloadURL ");
                    log(downloadURLc);                    
                }
            }
            return(results);
        } else {
            log("callFB alarm failed to parse JSON");
            results.success = false;
            return(results);
        }   
    }
    log("callFB sub connect Failure");
    results.subscribed = false;
    return(results);
}  
    