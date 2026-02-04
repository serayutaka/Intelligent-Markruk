#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <thread>
#include <chrono>
#include <vector>
#include "WString.h"

using byte = uint8_t;
using boolean = bool;

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1

// Mock Serial
class SerialMock {
public:
    void begin(long baud) { std::cout << "Serial started at " << baud << std::endl; }
    void print(const std::string& s) { std::cout << s; }
    void print(int n) { std::cout << n; }
    void print(long n) { std::cout << n; }
    void print(unsigned long n) { std::cout << n; }
    void println(const std::string& s) { std::cout << s << std::endl; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int n) { std::cout << n << std::endl; }
    void println(long n) { std::cout << n << std::endl; }
    void println(unsigned long n) { std::cout << n << std::endl; }
    void println() { std::cout << std::endl; }
    
    // Conversion operator to bool for "while (!Serial)" checks
    operator bool() const { return true; }
};

extern SerialMock Serial;

// Mock Time
inline unsigned long millis() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    return duration_cast<milliseconds>(steady_clock::now() - start).count();
}

inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void delayMicroseconds(unsigned int us) {
    // Determine if we really need to sleep for micro-ops in emulation (usually no)
    // std::this_thread::sleep_for(std::chrono::microseconds(us)); 
}

// Mock GPIO (No-ops, since we use high-level abstraction)
inline void pinMode(int pin, int mode) {}
inline void digitalWrite(int pin, int val) {}
inline int digitalRead(int pin) { return HIGH; } // Default high

// Mock Math
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))

