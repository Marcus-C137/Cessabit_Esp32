#ifndef CallFB
#define CallFB

struct callResults
{
    double setTemps[4] = {0.0,0.0,0.0,0.0};
    double lowAlarms[4] = {0.0,0.0,0.0,0.0};
    double highAlarms[4] = {0.0,0.0,0.0,0.0};
    bool alarmsMonitored[4] = {false, false, false, false};
    bool success = false;
    bool tempsChangedOnline = false;
    int newestVersion = 0;
    String downloadURL = "";
};
struct callParams
{
    String UID;
    String idToken;
    bool changedTempsOnScreen = false;
    double Temperatures[4] = {0.0,0.0,0.0,0.0};
    double localSetTemperatures[4] = {0.0,0.0,0.0,0.0};
    double localLowAlarms[4] = {0.0,0.0,0.0,0.0};
    double localHighAlarms[4] = {0.0,0.0,0.0,0.0};
    bool localAlarmsMonitored[4] = {false, false, false, false};
    String deviceID;
};
callResults callFB(callParams params, bool Debug);

#endif