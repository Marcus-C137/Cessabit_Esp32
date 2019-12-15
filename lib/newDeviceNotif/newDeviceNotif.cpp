#include <Arduino.h>  
#include <ArduinoJson.h>
#include "newDeviceNotif.h"
#include "globals.h"


bool newDeviceNotif(String UID, String deviceID, String idToken, bool Debug){

    String API_Key = "AIzaSyDOuF3bf6XnwzKZZoYX7tT_nV4JEI8IyNE";
    WiFiClientSecure client = WiFiClientSecure();


    DynamicJsonBuffer JSONbuffer;
    JsonObject& root = JSONbuffer.createObject();
    root["UID"] = UID;
    root["deviceID"] = deviceID;
    String JsonRequest;
    root.printTo(JsonRequest);
    if (Debug){
        char JSONmessageBuffer[300];
        root.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        Serial.println("JSONmessageBuffer: "); Serial.println(JSONmessageBuffer);
        Serial.println("JsonRequest: "); Serial.println(JsonRequest);
        Serial.println("Size of JsonRequest: ");Serial.println(sizeof(JsonRequest));
    }

    char firestoreFunction[] = "us-central1-zagermonitoringfb.cloudfunctions.net";
    
    if(client.connect(firestoreFunction,443)){
        
        if (Debug){
            Serial.println("newDeviceNotif FirestoreAPI connected");
        }

        String firestoreURL = "/addDevice";
        client.println("POST " + firestoreURL + " HTTP/1.1");
        client.print("Host: "); client.println(firestoreFunction);
        client.print("Authorization: Bearer "); client.println(idToken);
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
        while (client.available()) {
            char c = client.read();
            body += c;
        }
        if (Debug){
            Serial.println("newDeviceNotif Firestore API body ");
            Serial.println(body);
        }
        client.stop();
        yield();
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(body);
        if (root.success()) {

            if(root.containsKey("error")){
                const char* error = root["error"];
                Serial.println("newDeviceNotif error: ");
                Serial.println(error);
                return(false);
            }
            if (Debug){
                String root_print;
                root.printTo(root_print);
                Serial.println("newDeviceNotif root : "); Serial.println(root_print);
            }
            Serial.println("newDeviceNotif Success");
            return(true);

        } else {
            Serial.println("failed to parse JSON at newDeviceNotif");
            return(false);
        }
        
    }
    Serial.println("newDeviceNotif Failure");
    return(false);
}  