#include <Arduino.h>
#include <ArduinoJson.h>
#include "Login.h"
#include "globals.h"

signInResults getSignInKey(String email, String password, bool Debug){

    struct signInResults result;
    WiFiClientSecure client = WiFiClientSecure();

    DynamicJsonBuffer JSONbuffer;
    JsonObject& JSONencoder = JSONbuffer.createObject();
    JSONencoder["email"] = email;
    JSONencoder["password"] = password;
    JSONencoder["returnSecureToken"] = true;
    String JsonRequest;
    JSONencoder.printTo(JsonRequest);

    
    if (Debug){
        char JSONmessageBuffer[200];
        JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        log("JSONmessageBuffer: ");
        log(JSONmessageBuffer);
        log("JsonRequest: "); 
        log(JsonRequest);
    }

    char host[] = "www.googleapis.com";
    
    if (client.connect(host,443)){
        if (Debug){
            log("Login Connected");
        }
        String URL = "https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyPassword?key=AIzaSyDOuF3bf6XnwzKZZoYX7tT_nV4JEI8IyNE";
        client.println("POST " + URL + " HTTP/1.1");
        client.print("Host: "); client.println(host);
        client.println("Content-Type: x-www-form-urlencoded");  
        client.println("User-Agent: ESP8266/1.0");      
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
        log("Free Heap " + String(ESP.getFreeHeap()));
        if(Debug){
            log("body :");
            log(body);
        }
        
        client.stop();
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(body);
        if (root.success()) {

            if (Debug){
                char root_print[500];
                root.prettyPrintTo(root_print,sizeof(root_print));
                log("root : "); log(root_print);
            }

            if (root.containsKey("idToken")) {
                char idToken_print[500];
                root["idToken"].prettyPrintTo(idToken_print, sizeof(idToken_print));
                result.idToken = root["idToken"].as<String>();
                
                if(Debug){
                    log("Got ID token :");
                    log(idToken_print);
                }
                
                result.success = true;
                log("Login Success");
                return(result);

            } else{
                
                log("failed to parse JSON at Login.cpp");
                return(result);
            
            }

        }

    }

    if(Debug){
        log("Login Result :");
        log("idToken : "); log(result.idToken);
        log("success :"); log(result.success);
    }
    
    log("Login Failure");
    yield();
    return(result);
}
