#ifndef NextionLib
#define NextionLib

#include "Arduino.h"
#include "globals.h"
#include "main.h"

#define NEX_RET_EVENT_TOUCH_HEAD            (0x65) 

int currentPage = 0;
int portSelected = 99;
bool writeTemps = false;
bool initialized = false;
bool setChangedTempsTimer = false;
bool addToSetTemps = false;
bool addToLowAlarms = false;
bool addToHighAlarms = false;
bool subFromSetTemps = false;
bool subFromLowAlarms = false;
bool subFromHighAlarms = false;


String tempDisplaysTxt[4] = {"p1t1.txt","p1t2.txt","p1t3.txt","p1t4.txt"};
uint32_t time_nextionTempsLastUpdated = 0;
uint32_t time_nextionTempsUpdate = 1000;

Ticker focusOnScreensTick;

void sendCommand(String command){
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.print(command);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
    Serial2.write(0xFF);
}

void nextion_getValue(String item, String val){
    String command = "get " + item + "." + val;
    sendCommand(command);
}

void nextion_setValue(String item, String val){
    String command = item + "=" + "\"" + val + "\"";
    sendCommand(command);
}

void updateScreenTemps(){
    for(int i=0; i<4; i++){
        char val[16];
        if (Temperatures[i] == 0){
            nextion_setValue(tempDisplaysTxt[i],"N/A");
        }else{
            dtostrf(Temperatures[i], 5, 2,val);
            nextion_setValue(tempDisplaysTxt[i],val);
        }
    }
    if (wifiConnected){
        long rssi = WiFi.RSSI();
        //log("RSSI = " + String(rssi));
        if (rssi < -40) sendCommand("pic 410,0,9");
        if (rssi < -60) sendCommand("pic 410,0,8");
        if (rssi < -80) sendCommand("pic 410,0,7");
    }else{
        sendCommand("pic 410,0,6");
    }
}

void nextion_handleEvent(uint8_t pageNo, uint8_t id, uint8_t event){

    if (pageNo == 0){
        sendCommand("page 1");
        currentPage = 0;
    }

    if(pageNo == 1){
        log("recieved page 1 information");
        currentPage = 1;
        switch(id)
        {
            case 0x07:
                portSelected = 0;
                log("port Seleced " + String(portSelected));
                break;
            case 0x08:
                portSelected = 1;
                log("port Seleced " + String(portSelected));
                break;
            case 0x09:
                portSelected = 2;
                log("port Seleced " + String(portSelected));
                break;
            case 0x0A:
                portSelected = 3;
                log("port Seleced " + String(portSelected));
                break;
        }
    }

    if(pageNo==3){
        log("currentPage is 3");
        currentPage = 3;
        SPIFFS.remove("/config.json");
        wifiConnect();
    }

    if(pageNo==4){
        log("in page 4");
        setChangedTempsTimer = true;
        currentPage= pageNo;
        char setTempVal[5];
        char hAlarmVal[5];
        char lAlarmVal[5];
        dtostrf(setTemperatures[portSelected], 4, 1,setTempVal);
        dtostrf(lowAlarmsTemps[portSelected], 4, 1,hAlarmVal);
        dtostrf(highAlarmsTemps[portSelected], 4, 1,lAlarmVal);
        nextion_setValue("t0.txt",setTempVal);
        nextion_setValue("t1.txt",hAlarmVal);
        nextion_setValue("t2.txt",lAlarmVal);
        
    switch(id)
    {
        case 0x03:
            if (event == 1) subFromSetTemps = true; else subFromSetTemps = false;
            break;
        case 0x04:
            if (event == 1) subFromLowAlarms = true; else subFromLowAlarms = false;
            break;
        case 0x05:
            if (event == 1) subFromHighAlarms = true; else subFromHighAlarms = false;
            break;
        case 0x02:
            if (event == 1) addToSetTemps = true; else addToSetTemps = false;
            break;
        case 0x06:
            if (event == 1) addToLowAlarms = true; else addToLowAlarms = false;
            break;
        case 0x07:
            if (event == 1) addToHighAlarms = true; else addToHighAlarms = false;
            break;
    }
    }

};

void nextion_loop(){
    bool debug = true;
    static uint8_t __buffer[10];
    uint8_t pageNo;
    uint8_t id; 
    uint8_t event; 
    uint16_t i;
    uint8_t c;
    bool setScreenTimer = false;
    while(Serial2.available()>0){
        delay(10);
        c = Serial2.read();
        if (NEX_RET_EVENT_TOUCH_HEAD == c)
        {

            if(Serial2.available()>=6)
            {
                __buffer[0]=c;
                for(i=1;i<7;i++)
                {
                    __buffer[i] = Serial2.read();
                }
                __buffer[i]=0x00;
                if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6])
                {
                    
                    pageNo = __buffer[1];
                    id =  __buffer[2];
                    event =  __buffer[3];
                    if (debug){
                        log("pageNo :" + String(pageNo));
                        log("id :" + String(id));
                        log("event :" + String(event));
                    }
                    
                    setScreenTimer = true;
                    nextion_handleEvent(pageNo,id,event);

                }
            }
        }
    }
    if(!initialized){
     initialized = true;
     sendCommand("page 1");
    }
    
    if (currentPage == 1) updateScreenTemps();
    
    if (subFromSetTemps || subFromLowAlarms || subFromHighAlarms || addToSetTemps || addToLowAlarms || addToHighAlarms){
        setScreenTimer = true;
        changedTempsOnScreen = true;
        writeChangedTempsToSpiffs(changedTempsOnScreen);
    }
    if(setScreenTimer){
        log("focus On Screen Tick On");
        setScreenTimer = false;
        notTouchingScreen = false;
        focusOnScreensTick.detach();
        focusOnScreensTick.once(1, ISR_screenFocusDone);
    }
    
    if(currentPage ==1 && millis() - time_nextionTempsLastUpdated>time_nextionTempsUpdate){
        time_nextionTempsLastUpdated = millis();
        updateScreenTemps();
    }

    if(subFromSetTemps){
        char setTempVal[5];
        setTemperatures[portSelected] -= 0.5;
        dtostrf(setTemperatures[portSelected], 4, 1,setTempVal);
        nextion_setValue("t0.txt", setTempVal);
        writeTempstoSpiffs(setTemperatures, highAlarmsTemps, lowAlarmsTemps, alarmsMonitored);
    }

    if(subFromLowAlarms){
        char lAlarmVal[5];
        log("low Alarm - selected");
        lowAlarmsTemps[portSelected] -= 0.5;
        dtostrf(lowAlarmsTemps[portSelected], 4, 1,lAlarmVal);
        nextion_setValue("t1.txt", lAlarmVal);
        writeTemps = true;
    }

    if(subFromHighAlarms){
        char hAlarmVal[5];
        log("high Alarm - selected");
        highAlarmsTemps[portSelected] -= 0.5;
        dtostrf(highAlarmsTemps[portSelected], 4, 1,hAlarmVal);
        nextion_setValue("t2.txt", hAlarmVal);
        writeTempstoSpiffs(setTemperatures, highAlarmsTemps, lowAlarmsTemps, alarmsMonitored);
    }

    if(addToSetTemps){
        char setTempVal[5];
        log("set Temp + selected");
        setTemperatures[portSelected] += 0.5;
        dtostrf(setTemperatures[portSelected], 4, 1,setTempVal);
        nextion_setValue("t0.txt", setTempVal);
        writeTempstoSpiffs(setTemperatures, highAlarmsTemps, lowAlarmsTemps, alarmsMonitored);
    }

    if(addToLowAlarms){
        char lAlarmVal[5];
        log("low Alarm + selected");
        lowAlarmsTemps[portSelected] += 0.5;
        dtostrf(lowAlarmsTemps[portSelected], 4, 1,lAlarmVal);
        nextion_setValue("t1.txt", lAlarmVal);
        writeTempstoSpiffs(setTemperatures, highAlarmsTemps, lowAlarmsTemps, alarmsMonitored);
    }

    if(addToHighAlarms){
        char hAlarmVal[5];
        log("high Alarm + selected");
        highAlarmsTemps[portSelected] += 0.5;
        dtostrf(highAlarmsTemps[portSelected], 4, 1,hAlarmVal);
        nextion_setValue("t2.txt", hAlarmVal);
        writeTempstoSpiffs(setTemperatures, highAlarmsTemps, lowAlarmsTemps, alarmsMonitored);
    }
    
}

int getCurrentPage(){
    return currentPage;
}

#endif