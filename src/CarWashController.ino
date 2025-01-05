#define TINY_GSM_RX_BUFFER          1024 // Set RX buffer to 1Kb

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#include "mqtt_lte_client.h"
#include "utilities.h"

MqttLteClient mqttClient;
const char *subscribe_topic = "GsmMqttTest/subscribe";
const char *publish_topic = "GsmMqttTest/publish";

void mqtt_callback(const char *topic, const uint8_t *payload, uint32_t len) {
    Serial.println("\n======mqtt_callback======");
    Serial.print("Topic: "); 
    Serial.println(topic);
    Serial.print("Payload: ");
    for (uint32_t i = 0; i < len; ++i) {
        Serial.print((char)payload[i]);
    }
    Serial.println("\n=========================");
}

void setup() {
    Serial.begin(115200);
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    mqttClient.begin(SerialAT);
    if (!mqttClient.connect()) {
        Serial.println("Failed to connect to MQTT broker");
        return;
    }
    mqttClient.setCallback(mqtt_callback);
    mqttClient.subscribe(subscribe_topic);
    
    Serial.println("Connected to MQTT broker");
}

void loop() {
    mqttClient.handle();
    
    // Debug AT commands
    if (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    if (Serial.available()) {
        SerialAT.write(Serial.read());
    }
    delay(1);
}


