#define TINY_GSM_RX_BUFFER          1024 // Set RX buffer to 1Kb

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#include "mqtt_lte_client.h"
#include "utilities.h"
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <algorithm>
#include "domain.h"
#include "constants.h"
#include "car_wash_controller.h"

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


