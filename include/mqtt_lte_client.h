#pragma once

#include "utilities.h"
#include <TinyGsmClient.h>
#include "certs/AWSClientCertificate.h"
#include "certs/AWSClientPrivateKey.h"
#include "certs/AmazonRootCA.h"

class MqttLteClient {
public:
    MqttLteClient();
    void begin(Stream &serial);
    bool connect();
    void handle();
    bool isConnected();
    void setCallback(void (*callback)(const char*, const uint8_t*, uint32_t));
    bool publish(const char* topic, const char* payload, const uint8_t qos);
    bool subscribe(const char* topic);

private:
    bool initModem();
    bool setupNetwork();
    bool connectMqtt();
    bool checkSimCard();
    
    TinyGsm* modem;
    static const char* BROKER;
    static const uint16_t BROKER_PORT;
    static const char* CLIENT_ID;
    static const char* NETWORK_APN;
    static const uint8_t MQTT_CLIENT_ID = 0;
    uint32_t lastCheckConnect = 0;
    bool isInitialized = false;
};