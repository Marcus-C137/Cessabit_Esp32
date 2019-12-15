#include <Arduino.h>
#ifndef LOGIN
#define LOGIN
struct signInResults
{
    String idToken = "";
    bool success = false;
};

signInResults getSignInKey(String email, String password, bool Debug);

#endif