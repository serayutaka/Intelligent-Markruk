#pragma once
#include "Arduino.h"

#define WL_CONNECTED 3
#define WL_IDLE_STATUS 0
#define WL_NO_MODULE 255

class WiFiClass {
public:
    void begin(const char* ssid, const char* pass) {}
    int status() { return WL_CONNECTED; }
    String localIP() { return "127.0.0.1"; }
};

inline WiFiClass WiFi;
