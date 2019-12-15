#ifndef subCallFB
#define subCallFB


struct callSubResults 
{
    bool success = false;
    bool subscribed = false;
    unsigned long latestVersion = 0;
    String DownloadURL;
};
callSubResults callFBsub(String idToken, bool Debug);

#endif