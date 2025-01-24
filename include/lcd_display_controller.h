// #pragma once
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include "domain.h"
// // #include "machine_config.h"

// class LcdDisplayController {
// public:
//     LcdDisplayController(uint8_t lcd_addr = 0x27, uint8_t lcd_cols = 20, uint8_t lcd_rows = 4);
    
//     void update(MachineState state, const MachineConfig& config, unsigned long remainingSeconds);

// private:
//     LiquidCrystal_I2C lcd;
//     MachineState currentState;
//     int tokens;
//     String userId;
//     String userName;
//     unsigned long secondsLeft;
//     unsigned long lastUpdateTime;
//     unsigned long updateInterval;
//     int activeButton;

//     void displayStatus();
//     void displayFreeState();
//     void displayIdleState();
//     void displayRunningState();
//     void displayPausedState();
//     void displayUnknownState();
//     String formatSeconds(unsigned long totalSeconds);
// };