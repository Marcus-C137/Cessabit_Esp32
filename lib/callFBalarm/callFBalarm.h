#ifndef alarmCallFB
#define alarmCallFB


struct alarmCallParams
{
    String idToken;
    double Temperatures[4] = {0.0, 0.0, 0.0, 0.0};
    bool AlarmMask[4] = {false, false, false, false};
};

bool callFBalarm(alarmCallParams params, bool Debug);

#endif