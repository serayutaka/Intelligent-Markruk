#include "Arduino.h"

// Define ENABLE_WIFI to false/undef to force local mode as per OpenChess.ino logic
#undef ENABLE_WIFI

// Include the sketch file directly
#include "OpenChess.ino"

int main() {
    // Run setup once
    setup();
    
    // Run loop forever
    while (1) {
        loop();
        // Sleep to save CPU
        delay(10);
    }
    return 0;
}
