#pragma once
#include "Arduino.h"

class WiFiSSLClient {
public:
    int connect(const char* host, uint16_t port) { return 1; }
    void print(String s) {}
    void println(String s = "") {}
    String readStringUntil(char c) { return ""; }
    bool connected() { return true; }
    void stop() {}
    int available() { return 0; }
    char read() { return 0; }
};
