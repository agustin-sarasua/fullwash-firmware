// #include "lcd_display_controller.h"

// LcdDisplayController::LcdDisplayController(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows)
//     : lcd(lcd_addr, lcd_cols, lcd_rows), 
//       currentState(STATE_FREE),
//       tokens(0),
//       userId(""),
//       userName(""),
//       secondsLeft(0),
//       lastUpdateTime(0),
//       updateInterval(1000),
//       activeButton(-1)
// {
//     lcd.init();
//     lcd.backlight();
//     lcd.clear();
// }

// void LcdDisplayController::update(MachineState state, const MachineConfig& config, unsigned long remainingSeconds) {
//     unsigned long currentTime = millis();
    
//     if (currentTime - lastUpdateTime < updateInterval) {
//         return;
//     }
    
//     currentState = state;
//     tokens = config.tokens;
//     userId = config.userId;
//     userName = config.userName;
//     secondsLeft = remainingSeconds;

//     displayStatus();
    
//     lastUpdateTime = currentTime;
// }

// void LcdDisplayController::displayStatus() {
//     lcd.clear();

//     switch (currentState) {
//         case STATE_FREE:
//             displayFreeState();
//             break;
//         case STATE_IDLE:
//             displayIdleState();
//             break;
//         case STATE_RUNNING:
//             displayRunningState();
//             break;
//         case STATE_PAUSED:
//             displayPausedState();
//             break;
//         default:
//             displayUnknownState();
//             break;
//     }
// }

// void LcdDisplayController::displayFreeState() {
//     lcd.setCursor(0, 0);
//     lcd.print("STATUS: FREE");
//     lcd.setCursor(0, 1);
//     lcd.print("Waiting for user...");
// }

// void LcdDisplayController::displayIdleState() {
//     lcd.setCursor(0, 0);
//     lcd.print("STATUS: READY");
    
//     lcd.setCursor(0, 1);
//     lcd.print("Tokens: " + String(tokens));
    
//     lcd.setCursor(0, 2);
//     lcd.print("User: " + (userName.length() > 0 ? userName : "N/A"));
// }

// void LcdDisplayController::displayRunningState() {
//     lcd.setCursor(0, 0);
//     lcd.print("STATUS: RUNNING");
    
//     lcd.setCursor(0, 1);
//     // lcd.print("Button: " + String(activeButton + 1));
    
//     lcd.setCursor(0, 2);
//     lcd.print("Time Left: " + formatSeconds(secondsLeft));
    
//     lcd.setCursor(0, 3);
//     lcd.print("Tokens: " + String(tokens));
// }

// void LcdDisplayController::displayPausedState() {
//     lcd.setCursor(0, 0);
//     lcd.print("STATUS: PAUSED");
    
//     lcd.setCursor(0, 1);
//     // lcd.print("Button: " + String(activeButton + 1));
    
//     lcd.setCursor(0, 2);
//     lcd.print("Time Left: " + formatSeconds(secondsLeft));
// }

// void LcdDisplayController::displayUnknownState() {
//     lcd.setCursor(0, 0);
//     lcd.print("STATUS: UNKNOWN");
//     lcd.setCursor(0, 1);
//     lcd.print("System Error");
// }

// String LcdDisplayController::formatSeconds(unsigned long totalSeconds) {
//     unsigned long minutes = totalSeconds / 60;
//     unsigned long seconds = totalSeconds % 60;
    
//     char buffer[10];
//     snprintf(buffer, sizeof(buffer), "%02lu:%02lu", minutes, seconds);
//     return String(buffer);
// }