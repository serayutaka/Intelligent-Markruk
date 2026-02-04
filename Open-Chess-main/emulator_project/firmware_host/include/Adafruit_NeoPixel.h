#pragma once
#include <cstdint>

#define NEO_GRB  0
#define NEO_KHZ800 0
#define NEO_GRBW 0

class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(uint16_t n, uint8_t p, uint8_t t) {}
    void begin() {}
    void show() {}
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {}
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {}
    void setPixelColor(uint16_t n, uint32_t c) {}
    void setBrightness(uint8_t t) {}
    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) { return 0; }
    uint32_t getPixelColor(uint16_t n) const { return 0; }
    void clear() {}
};
