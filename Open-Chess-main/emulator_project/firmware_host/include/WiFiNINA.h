#pragma once
#include "Arduino.h"

#define WL_CONNECTED 3
#define WL_IDLE_STATUS 0

class WiFiClass {
public:
    void begin(const char* ssid, const char* pass) {}
    int status() { return WL_CONNECTED; }
};

inline WiFiClass WiFi;
