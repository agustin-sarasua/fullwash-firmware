#define TINY_GSM_RX_BUFFER          1024 // Set RX buffer to 1Kb

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#include "mqtt_lte_client.h"
// #include "utilities.h"
#include <TinyGsmClient.h>
// #include "certs/AWSClientCertificate.h"
// #include "certs/AWSClientPrivateKey.h"
// #include "certs/AmazonRootCA.h"
#include <StreamDebugger.h>

const char* MqttLteClient::BROKER = "a3foc0mc6v7ap0-ats.iot.us-east-1.amazonaws.com";
const uint16_t MqttLteClient::BROKER_PORT = 8883;
const char* MqttLteClient::CLIENT_ID = "fullwash-machine-001";
const char* MqttLteClient::NETWORK_APN = "internet";

#define NETWORK_APN           "internet"

MqttLteClient::MqttLteClient() : modem(nullptr) {}

void MqttLteClient::begin(Stream &serial) {
    #ifdef DUMP_AT_COMMANDS
    static StreamDebugger debugger(serial, Serial);
    modem = new TinyGsm(debugger);
    #else
    modem = new TinyGsm(serial);
    #endif

    // Initialize pins
    #ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
    #endif

    pinMode(MODEM_RESET_PIN, OUTPUT);
    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    
    // Reset sequence
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);
    delay(100);
    digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL);
    delay(2600);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    delay(100);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    
    isInitialized = initModem() && checkSimCard() && setupNetwork();
}

bool MqttLteClient::initModem() {
    int retry = 0;
    while (!modem->testAT(1000)) {
        Serial.print(".");
        if (retry++ > 10) {
            digitalWrite(BOARD_PWRKEY_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_PWRKEY_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_PWRKEY_PIN, LOW);
            retry = 0;
        }
    }
    return true;
}

bool MqttLteClient::checkSimCard() {
    SimStatus sim = SIM_ERROR;
    while (sim != SIM_READY) {
        sim = modem->getSimStatus();
        if (sim == SIM_LOCKED) {
            const char *SIMCARD_PIN_CODE = "3846";
            modem->simUnlock(SIMCARD_PIN_CODE);
        }
        delay(1000);
    }
    return true;
}

bool MqttLteClient::setupNetwork() {
    #ifndef TINY_GSM_MODEM_SIM7672
    modem->setNetworkMode(MODEM_NETWORK_AUTO);
    #endif

    #ifdef NETWORK_APN
    modem->sendAT(GF("+CGDCONT=1,\"IP\",\""), NETWORK_APN, "\"");
    modem->waitResponse();
    #endif

    RegStatus status;
    do {
        status = modem->getRegistrationStatus();
        if (status == REG_DENIED) {
            return false;
        }
        delay(1000);
    } while (status != REG_OK_HOME && status != REG_OK_ROAMING);

    if (!modem->setNetworkActive()) {
        return false;
    }

    delay(5000);
    modem->mqtt_begin(true);
    modem->mqtt_set_certificate(AmazonRootCA, AWSClientCertificate, AWSClientPrivateKey);
    
    return true;
}

bool MqttLteClient::connectMqtt() {
    if (!BROKER || !CLIENT_ID) {
        Serial.println("Invalid MQTT broker or client ID");
        return false;
    }
    
    // Serial.println("Connecting to MQTT broker");
    // Serial.print("Broker: ");
    // Serial.println(BROKER);
    // Serial.print("Port: ");
    // Serial.println(BROKER_PORT);
    // Serial.print("Client ID: ");
    // Serial.println(CLIENT_ID);
    modem->setWillMessage("GsmMqttTest/lastwill", "offline", 0);

    if (!modem->mqtt_connect(MQTT_CLIENT_ID, BROKER, BROKER_PORT, CLIENT_ID)) {
        return false;
    }
    return modem->mqtt_connected();
}

bool MqttLteClient::connect() {
    if (!isInitialized) return false;
    return connectMqtt();
}

void MqttLteClient::handle() {
    if (!isInitialized) return;
    
    if (millis() > lastCheckConnect) {
        lastCheckConnect = millis() + 10000UL;
        if (!modem->mqtt_connected()) {
            connectMqtt();
        }
    }
    modem->mqtt_handle();
}

bool MqttLteClient::isConnected() {
    return modem->mqtt_connected();
}

void MqttLteClient::setCallback(void (*callback)(const char*, const uint8_t*, uint32_t)) {
    modem->mqtt_set_callback(callback);
}

bool MqttLteClient::publish(const char* topic, const char* payload) {
    return modem->mqtt_publish(MQTT_CLIENT_ID, topic, payload);
}

bool MqttLteClient::subscribe(const char* topic) {
    return modem->mqtt_subscribe(MQTT_CLIENT_ID, topic);
}