#include "board_driver.h"
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include "Arduino.h"

// Global shadow state
static bool shadowSensors[8][8];
static std::mutex sensorMutex;

// Socket globals
static int sock = -1;
static std::thread receiverThread;
static bool running = true;

// Helper to send command
static void sendCmd(const std::string& cmd) {
    if (sock >= 0) {
        std::string msg = cmd + "\n";
        send(sock, msg.c_str(), msg.length(), 0);
    }
}

BoardDriver::BoardDriver() : strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800) {
    for (int r=0; r<8; r++)
       for (int c=0; c<8; c++) 
           shadowSensors[r][c] = false;
}

void BoardDriver::begin() {
    Serial.println("Emulator Wrapper: Connecting to localhost:2323...");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(2323);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        Serial.println("Connection Failed to Emulator (127.0.0.1:2323).");
    } else {
        Serial.println("Connected to Emulator!");
        
        receiverThread = std::thread([]() {
            char buffer[4096];
            while (running && sock >= 0) {
                int len = recv(sock, buffer, 4096 - 1, 0);
                if (len > 0) {
                    buffer[len] = 0;
                    char* line = strtok(buffer, "\n");
                    while(line) {
                        // Protocol: E <row> <col> <1|0>
                        if (strncmp(line, "E ", 2) == 0) {
                            int r, c, s;
                            if (sscanf(line, "E %d %d %d", &r, &c, &s) == 3) {
                                if (r >= 0 && r < 8 && c >= 0 && c < 8) {
                                    std::lock_guard<std::mutex> lock(sensorMutex);
                                    shadowSensors[r][c] = (s == 1);
                                    // Debug
                                    // std::cout << "Sensor " << r << "," << c << " = " << s << std::endl;
                                }
                            }
                        }
                        line = strtok(NULL, "\n");
                    }
                } else {
                    Serial.println("Disconnected from Emulator.");
                    running = false;
                    break;
                }
            }
        });
        receiverThread.detach();
    }
}

void BoardDriver::readSensors() {
    std::lock_guard<std::mutex> lock(sensorMutex);
    for (int r=0; r<8; r++) {
        for (int c=0; c<8; c++) {
            sensorState[r][c] = shadowSensors[r][c];
        }
    }
}

bool BoardDriver::getSensorState(int row, int col) {
    return sensorState[row][col];
}

bool BoardDriver::getSensorPrev(int row, int col) {
    return sensorPrev[row][col];
}

void BoardDriver::updateSensorPrev() {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            sensorPrev[row][col] = sensorState[row][col];
        }
    }
}

void BoardDriver::clearAllLEDs() {
    sendCmd("C");
}

void BoardDriver::setSquareLED(int row, int col, uint32_t color) {
    uint8_t r = (uint8_t)(color >> 16);
    uint8_t g = (uint8_t)(color >> 8);
    uint8_t b = (uint8_t)color;
    // Assume no W for now? Or just ignore it? 
    // Protocol L r c r g b
    char cmd[64];
    sprintf(cmd, "L %d %d %d %d %d", row, col, r, g, b);
    sendCmd(cmd);
}

void BoardDriver::setSquareLED(int row, int col, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    // Protocol L r c r g b
    // W is unsupported in simple protocol for now, can add later
    char cmd[64];
    sprintf(cmd, "L %d %d %d %d %d", row, col, r, g, b);
    sendCmd(cmd);
}

void BoardDriver::showLEDs() {
    sendCmd("S");
}

// Animations - just delegate or simplify?
// Original impl does loops with delays.
// We can reproduce them or just send rapid commands. 
// For now, let's keep the logic if we can, but it might be slow over TCP?
// Actually, fireworkAnimation calls setSquareLED many times.
// Mocking the behavior is safer.

void BoardDriver::fireworkAnimation() {
    // Just a placeholder for now
    Serial.println("Firework Animation Triggered");
}

void BoardDriver::captureAnimation(int row, int col) {
    Serial.println("Capture Animation Triggered");
    blinkSquare(row, col, 2);
}

void BoardDriver::promotionAnimation(int col) {
    Serial.println("Promotion Animation Triggered");
}

void BoardDriver::blinkSquare(int row, int col, int times) {
    for(int i=0; i<times; i++) {
        setSquareLED(row, col, 255, 0, 0); // Red blink
        showLEDs();
        delay(200);
        setSquareLED(row, col, 0, 0, 0);
        showLEDs();
        delay(200);
    }
}

void BoardDriver::highlightSquare(int row, int col, uint32_t color) {
    setSquareLED(row, col, color);
}

// Helper methods from original (copied because private access needed if implemented differently, but here we just impl the interface)

bool BoardDriver::checkInitialBoard(const char initialBoard[8][8]) {
    // Need to read sensors
    readSensors();
    bool correct = true;
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            bool hasPiece = (initialBoard[r][c] != ' ');
            if (sensorState[r][c] != hasPiece) correct = false;
        }
    }
    return correct;
}

void BoardDriver::updateSetupDisplay(const char initialBoard[8][8]) {
    // Light up wrong squares?
    // Reuse specific logic if needed, or just:
    clearAllLEDs();
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
             bool hasPiece = (initialBoard[r][c] != ' ');
             if (sensorState[r][c] != hasPiece) {
                 setSquareLED(r, c, 255, 0, 0); // Red for error
             } else if (hasPiece) {
                 setSquareLED(r, c, 0, 255, 0); // Green for OK
             }
        }
    }
    showLEDs();
}

void BoardDriver::printBoardState(const char initialBoard[8][8]) {
    // Debug print
}

// internal helpers unused here
void BoardDriver::loadShiftRegister(byte data) {}
int BoardDriver::getPixelIndex(int row, int col) { return 0; }
