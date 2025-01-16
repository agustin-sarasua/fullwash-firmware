#define TINY_GSM_RX_BUFFER          1024 // Set RX buffer to 1Kb

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#include "mqtt_lte_client.h"
#include "utilities.h"
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <algorithm>
#include "domain.h"

// Pin Definitions
const int NUM_BUTTONS = 1;
const int STOP_BUTTON_PIN = 32;

// const int BUTTON_PINS[] = {14, 15, 16, 17, 18};
// const int BUTTON_PINS[] = {21, 22, 23};
const int BUTTON_PINS[] = {22};
// const int RELAY_PINS[] = {33, 34, 35};

const int LED_PIN_INIT = 18;
const int RUNNING_LED_PIN = 21;

const unsigned long STATE_RUNNING_TIME = 120000; // 2 minutes in milliseconds
const unsigned long TOKEN_TIME = 120000;   // Time per token (2 minutes)

// MQTT Topics
const char* MACHINE_ID = "99";

// Function to create the subscribe topic
String buildTopicName(const char* machineId, const char* eventType) {
    return String("machines/") + machineId + "/" + eventType;
}

// Subscribe
const String INIT_TOPIC = buildTopicName(MACHINE_ID, "init");
const String CONFIG_TOPIC = buildTopicName(MACHINE_ID, "config");
// Publish
const String ACTION_TOPIC = buildTopicName(MACHINE_ID, "action");
const String STATE_TOPIC = buildTopicName(MACHINE_ID, "state");

// QoS 0 - At most once delivery
// This quality of service ensures that the message arrives at the receiver at most once.

const uint32_t QOS0_AT_MOST_ONCE = 0;
// QoS 1 - At least once delivery
// This quality of service ensures that the message arrives at the receiver at least once.
const uint32_t QOS1_AT_LEAST_ONCE = 1;

// TIMOUT 2 mins
const unsigned long USER_INACTIVE_TIMEOUT = 120000;

// Machine Configuration
struct MachineConfig {
    String sessionId;
    String userId;
    String userName;
    int tokens;
    String timestamp;
    bool isLoaded;
};

class CarWashController {
    
public:
    CarWashController(MqttLteClient& client):
        mqttClient(client),
        currentState(STATE_FREE),
        lastActionTime(0),
        activeButton(-1),
        tokenStartTime(0),
        lastStatePublishTime(0),
        tokenTimeElapsed(0),
        pauseStartTime(0) {
        
        // LED
        pinMode(LED_PIN_INIT, OUTPUT);
        digitalWrite(LED_PIN_INIT, LOW);

        pinMode(RUNNING_LED_PIN, OUTPUT);
        digitalWrite(RUNNING_LED_PIN, LOW);

        // Initialize pins (same as before)
        for (int i = 0; i < NUM_BUTTONS; i++) {
            pinMode(BUTTON_PINS[i], INPUT_PULLUP);
            // pinMode(RELAY_PINS[i], OUTPUT);
            // digitalWrite(RELAY_PINS[i], LOW);
            lastDebounceTime[i] = 0;
            lastButtonState[i] = HIGH;
        }
        
        pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
        lastDebounceTime[NUM_BUTTONS] = 0;
        lastButtonState[NUM_BUTTONS] = HIGH;
        
        config.isLoaded = false;
    }
    
    void handleMqttMessage(const char* topic, const uint8_t* payload, uint32_t len) {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload, len);
        if (error) {
            Serial.println("Failed to parse JSON");
            return;
        }
        if (String(topic) == INIT_TOPIC) {            
            // Update machine configuration
            config.sessionId = doc["session_id"].as<String>();
            config.userId = doc["user_id"].as<String>();
            config.userName = doc["user_name"].as<String>();
            config.tokens = doc["tokens"].as<int>();
            config.timestamp = doc["timestamp"].as<String>();
            config.isLoaded = true;
            currentState = STATE_IDLE;
            lastActionTime = millis();
            // // Switch on LED
            Serial.println("Switching on LED");
            digitalWrite(LED_PIN_INIT, HIGH);

            Serial.println("Machine loaded with new configuration");
        } else if (String(topic) == CONFIG_TOPIC) {
            Serial.print("Updating machine configuration timestamp: ");
            Serial.println(doc["timestamp"].as<String>());
            config.timestamp = doc["timestamp"].as<String>();
            // Reset all other config values
            config.sessionId = "";
            config.userId = "";
            config.userName = "";
            config.tokens = 0;
            config.isLoaded = false;
        } else {
            Serial.print("Unknown topic: ");
            Serial.println(String(topic));
        }
        
    }
    
   
    void handleButtons() {
        // Handle regular buttons
        for (int i = 0; i < NUM_BUTTONS; i++) {
            int reading = digitalRead(BUTTON_PINS[i]);
            
            if (reading != lastButtonState[i]) {
                lastDebounceTime[i] = millis();
            }
            
            if ((millis() - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
                // Serial.print("Button reading: ");   
                // Serial.println(String(reading));
                if (reading == LOW && config.isLoaded) {
                    Serial.print("Button pressed: ");
                    Serial.println(String(i));
                    if (currentState == STATE_IDLE) {
                        activateButton(i, MANUAL);
                    } else if (currentState == STATE_RUNNING && i == activeButton) {
                        pauseMachine();
                    } else if (currentState == STATE_PAUSED) {
                        resumeMachine(i);
                    }
                }
            }
            
            lastButtonState[i] = reading;
        }
        
        // Handle stop button
        int stopReading = digitalRead(STOP_BUTTON_PIN);
        if (stopReading != lastButtonState[NUM_BUTTONS]) {
            lastDebounceTime[NUM_BUTTONS] = millis();
        }
        
        if ((millis() - lastDebounceTime[NUM_BUTTONS]) > DEBOUNCE_DELAY) {
            if (stopReading == LOW && currentState == STATE_RUNNING) {
                stopMachine(MANUAL);
            }
        }
        
        lastButtonState[NUM_BUTTONS] = stopReading;
    }

    void pauseMachine() {
        Serial.println("Pausing machine");
        if (activeButton >= 0) {
    //         digitalWrite(RELAY_PINS[activeButton], LOW);
        }
        currentState = STATE_PAUSED;
        lastActionTime = millis();
        pauseStartTime = millis();  // Record when we paused

        // Calculate elapsed token time up to this point
        tokenTimeElapsed += (pauseStartTime - tokenStartTime);
        digitalWrite(RUNNING_LED_PIN, LOW);
    }
    
    void resumeMachine(int buttonIndex) {
        Serial.println("Resuming machine");
        // digitalWrite(RELAY_PINS[activeButton], LOW);  // Turn off previous relay if different button
        activeButton = buttonIndex;
        // digitalWrite(RELAY_PINS[buttonIndex], HIGH);
        currentState = STATE_RUNNING;
        lastActionTime = millis();
        tokenStartTime = millis();  // Reset the token start time
        digitalWrite(RUNNING_LED_PIN, HIGH);
    }
    
    void stopMachine(TriggerType triggerType = AUTOMATIC) {
        if (activeButton >= 0) {
            // digitalWrite(RELAY_PINS[activeButton], LOW);
        }
        
        config.isLoaded = false;
        
        currentState = STATE_FREE;
        // Switch on LED
        digitalWrite(LED_PIN_INIT, LOW);
        activeButton = -1;
        tokenStartTime = 0;
        tokenTimeElapsed = 0;  // Reset elapsed time
        pauseStartTime = 0;
    }
    
    void activateButton(int buttonIndex, TriggerType triggerType = MANUAL) {
        Serial.print("Activating button: ");
        Serial.println(String(BUTTON_PINS[buttonIndex]));

        if (config.tokens <= 0) return;

        digitalWrite(RUNNING_LED_PIN, HIGH);
        currentState = STATE_RUNNING;
        activeButton = buttonIndex;
        // digitalWrite(RELAY_PINS[buttonIndex], HIGH);
        lastActionTime = millis();
        tokenStartTime = millis();
        tokenTimeElapsed = 0;  // Reset elapsed time for new token
        config.tokens--;
        
        publishActionEvent(buttonIndex, ACTION_START, triggerType);
    }

    void tokenExpired() {
        if (activeButton >= 0) {
            // digitalWrite(RELAY_PINS[activeButton], LOW);
        }
        activeButton = -1;
        // config.tokens--;
        currentState = STATE_IDLE;
        digitalWrite(RUNNING_LED_PIN, LOW);
        // publishTokenExpiredEvent();
    }
    
    void update() {
        if (config.timestamp == "") {
            return;
        }
        handleButtons();
        unsigned long currentTime = millis();
        publishPeriodicState();
        if (currentState == STATE_RUNNING) {
            // Calculate total token time including previous elapsed time
            unsigned long totalElapsedTime = tokenTimeElapsed + (currentTime - tokenStartTime);

            // Check if total token time is expired
            if (totalElapsedTime >= TOKEN_TIME) {
                Serial.println("Token expired");
                tokenExpired();
                return;
            }
            
        }
        // Check for user inactivity
        if (currentTime - lastActionTime > USER_INACTIVE_TIMEOUT && currentState != STATE_FREE) {
            Serial.println("Stopping machine due to user inactivity");
            stopMachine(AUTOMATIC);
        }
    }

    void publishMachineSetupActionEvent() {
        Serial.println("Publishing Machine Setup Action Event");

        StaticJsonDocument<512> doc;
        doc["machine_id"] = MACHINE_ID;
        doc["action"] = getMachineActionString(ACTION_SETUP);
        doc["timestamp"] = getTimestamp();
        
        String jsonString;
        serializeJson(doc, jsonString);
        mqttClient.publish(ACTION_TOPIC.c_str(), jsonString.c_str(), QOS1_AT_LEAST_ONCE);
    }
    
private:
    MqttLteClient& mqttClient;
    MachineState currentState;
    MachineConfig config;
    
    unsigned long lastActionTime;
    
    unsigned long tokenStartTime;   // Track when current token started
    int activeButton;

    unsigned long tokenTimeElapsed;    // Total time elapsed for current token
    unsigned long pauseStartTime;      // When the machine was paused

    // Button debouncing
    static const unsigned long DEBOUNCE_DELAY = 50;
    unsigned long lastDebounceTime[NUM_BUTTONS + 1];
    int lastButtonState[NUM_BUTTONS + 1];


    unsigned long lastStatePublishTime;
    const unsigned long STATE_PUBLISH_INTERVAL = 10000; // 10 seconds

    unsigned long getSecondsLeft() {
        if (currentState != STATE_RUNNING && currentState != STATE_PAUSED) {
            return 0;
        }

        unsigned long currentTime = millis();
        unsigned long totalElapsedTime;

        if (currentState == STATE_RUNNING) {
            totalElapsedTime = tokenTimeElapsed + (currentTime - tokenStartTime);
        } else { // PAUSED
            totalElapsedTime = tokenTimeElapsed;
        }

        if (totalElapsedTime >= TOKEN_TIME) {
            return 0;
        }

        return (TOKEN_TIME - totalElapsedTime) / 1000;
    }

    String getTimestamp() {
        if (config.timestamp == "")
            return "";

        // Variables to hold parsed values
        int year, month, day, hour, minute, second;
        float fractional_seconds;

        // Parse the server timestamp
        sscanf(config.timestamp.c_str(), "%d-%d-%dT%d:%d:%fZ",
            &year, &month, &day, &hour, &minute, &fractional_seconds);
        second = (int)fractional_seconds;

        // Use tmElements_t structure to calculate time
        tmElements_t tm;
        tm.Year = year - 1970; // Adjust to TimeLib's base year
        tm.Month = month;
        tm.Day = day;
        tm.Hour = hour;
        tm.Minute = minute;
        tm.Second = second;

        // Calculate server timestamp in seconds since epoch
        time_t serverEpoch = makeTime(tm);

        // Add current milliseconds since start
        unsigned long millisOffset = millis();
        time_t adjustedTime = serverEpoch + (millisOffset / 1000);
        int extraMilliseconds = millisOffset % 1000;

        // Convert adjusted time to components
        tmElements_t adjustedTm;
        breakTime(adjustedTime, adjustedTm);

        // Convert to ISO 8601 format
        char isoTimestamp[30];
        sprintf(isoTimestamp, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                adjustedTm.Year + 1970, adjustedTm.Month, adjustedTm.Day,
                adjustedTm.Hour, adjustedTm.Minute, adjustedTm.Second, extraMilliseconds);

        return String(isoTimestamp);
    }

    void publishActionEvent(int buttonIndex, MachineAction machineAction, TriggerType triggerType = MANUAL) {
        if (!config.isLoaded) return;
        
        StaticJsonDocument<512> doc;
        
        doc["machine_id"] = MACHINE_ID;
        doc["timestamp"] = getTimestamp();
        doc["action"] = getMachineActionString(machineAction);
        doc["trigger_type"] = (triggerType == MANUAL) ? "MANUAL" : "AUTOMATIC";
        doc["button_name"] = String("BUTTON_") + String(buttonIndex + 1);
        doc["session_id"] = config.sessionId;
        doc["user_id"] = config.userId;
        doc["token_channel"] = "PHYSICAL";
        doc["tokens_left"] = config.tokens;
        
        if (currentState == STATE_RUNNING) {
            unsigned long currentTime = millis();
            doc["seconds_left"] = getSecondsLeft();
        }
        
        String jsonString;
        serializeJson(doc, jsonString);
        
        mqttClient.publish(ACTION_TOPIC.c_str(), jsonString.c_str(), QOS1_AT_LEAST_ONCE);
    }

    void publishPeriodicState(bool force = false) {

        if (force || millis() - lastStatePublishTime >= STATE_PUBLISH_INTERVAL) {
            Serial.println("Publishing Periodic State");
            StaticJsonDocument<512> doc;
            doc["machine_id"] = MACHINE_ID;
            doc["timestamp"] = getTimestamp();
            
            // String(currentState));
            doc["status"] = getMachineStateString(currentState);
            Serial.print("Status: ");
            Serial.println(doc["status"].as<String>());
            
            // Add session metadata if available
            if (config.isLoaded) {
                JsonObject sessionMetadata = doc.createNestedObject("session_metadata");
                sessionMetadata["session_id"] = config.sessionId;
                sessionMetadata["user_id"] = config.userId;
                sessionMetadata["user_name"] = config.userName;
                sessionMetadata["tokens_left"] = config.tokens;
                sessionMetadata["timestamp"] = config.timestamp;
                if (tokenStartTime > 0) {
                    sessionMetadata["seconds_left"] = getSecondsLeft();
                }
            }
            
            String jsonString;
            serializeJson(doc, jsonString);
            bool result = mqttClient.publish(STATE_TOPIC.c_str(), jsonString.c_str(), QOS0_AT_MOST_ONCE);
            // Serial.print("Publishing MQTT Result: ");
            // Serial.println(String(result));
            
            lastStatePublishTime = millis();
        }
    }

};

// Global instances
MqttLteClient mqttClient;
CarWashController* controller;
static unsigned long lastHeapPrint = 0;

void mqtt_callback(const char *topic, const uint8_t *payload, uint32_t len) {
    if (controller) {
        controller->handleMqttMessage(topic, payload, len);
    }
}

void setup() {
    Serial.begin(115200);
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
    Serial.println("Starting everything");
    
    mqttClient.begin(SerialAT);
    if (!mqttClient.connect()) {
        Serial.println("Failed to connect to MQTT broker");
        return;
    }

    controller = new CarWashController(mqttClient);
    mqttClient.setCallback(mqtt_callback);
    mqttClient.subscribe(INIT_TOPIC.c_str());
    mqttClient.subscribe(CONFIG_TOPIC.c_str());
    controller->publishMachineSetupActionEvent();
    
    Serial.println("Connected to MQTT broker");
}

void loop() {
    // print ESP.getFreeHeap()
    // Print heap every 10 second
    if (millis() - lastHeapPrint > 10000) {
        lastHeapPrint = millis();
        Serial.print("Free Heap:");
        Serial.println(ESP.getFreeHeap());
    }

    mqttClient.handle();

    if (controller) {
        controller->update();
    }
    
    // Debug AT commands
    if (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    if (Serial.available()) {
        SerialAT.write(Serial.read());
    }
    delay(1);
}


