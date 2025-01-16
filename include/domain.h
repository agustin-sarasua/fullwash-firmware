#include <string>
#include <WString.h>

// Machine States
enum MachineState {
    STATE_FREE,
    
    STATE_IDLE, 
    STATE_RUNNING,
    STATE_PAUSED
    // OFFLINE
};

enum MachineAction {
    ACTION_SETUP,
    ACTION_START,
    ACTION_STOP,
    ACTION_PAUSE,
    ACTION_RESUME
};


String getMachineActionString(MachineAction action) {
    switch (action) {
        case ACTION_SETUP: return "SETUP";
        case ACTION_START: return "START";
        case ACTION_STOP: return "STOP";
        case ACTION_PAUSE: return "PAUSE";
        case ACTION_RESUME: return "RESUME";
        default: return "START";
    }
}

String getMachineStateString(MachineState state) {
    switch (state) {
        case STATE_FREE: return "FREE";
        case STATE_IDLE: return "IDLE";
        case STATE_RUNNING: return "RUNNING";
        case STATE_PAUSED: return "PAUSED";
        default: return "IDLE";
    }
}

enum TriggerType {
    MANUAL,
    AUTOMATIC
};

// enum TokenChannel {
//     PHYSICAL,
//     DIGITAL
// };

enum MachineButtonName {
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_5
};