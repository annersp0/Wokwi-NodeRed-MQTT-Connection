// ------------ Encapsulated Code; No changes needed ------------//
#include <WiFi.h>
#include "PubSubClient.h"

const char *ssid = "Wokwi-GUEST";
const char *password = "";

String stMac;
char mac[50];
char clientId[50];

WiFiClient espClient;
PubSubClient client(espClient);
// ------------ Encapsulated Code; No changes needed ------------//

const char *mqttServer = "mqtt.flespi.io";                                                  // MQTT Broker URL (no changes required)
const char *mqtt_user = "CP3hVZnjoAip8sfq1NSdqe8jBqLIskBUXu1JjfQvDWKBcPQ3qy5HA3O0hnyzyXjj"; // Update with your Flespi token
const char *mqtt_password = "";                                                             // Enter your Flespi password (below the token)
int port = 1883;                                                                            // MQTT port settings (do not modify)

const char *topicFromNodered = "Lastname1/Lastname2/Lastname3/fromNodered"; // Replace with your last names
const char *topicFromWokwi = "Lastname1/Lastname2/Lastname3/fromWokwi"; // Replace with your last names

// Adjust based on your component connections
const int PIRPin = 19; 
const int LED = 5;
const int buzzPin = 18;
const int soundFrequency = 262;

// Initial setup: Configures serial communication, Wi-Fi, MQTT, and pin modes
void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(0));

    Serial.print("Connecting to ");
    Serial.println(ssid);

    wifiConnect();

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    uint8_t mac[6];
    WiFi.macAddress(mac);
    for (int i = 0; i < 6; ++i){
      Serial.print(mac[i], HEX);
      if (i < 5)Serial.print(":");
    }
    Serial.println();

    client.setServer(mqttServer, port);
    client.setCallback(callback);

    pinMode(PIRPin, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(buzzPin, OUTPUT);
}

// Connect to Wokwi's virtual Wi-Fi for VPN creation
void wifiConnect() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
}

void mqttReconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println(" connected");
            client.subscribe(topicFromNodered);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(500);
        }
    }
}

// Callback function to handle incoming MQTT messages
void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String stMessage;

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        stMessage += (char)message[i];
    }
    Serial.println();
}

// Handles MQTT connection, PIR state changes, LED and Buzzer control
void loop() {
    if (!client.connected()) {
        mqttReconnect();
    }
    client.loop();

    static bool lastPIRState = LOW;
    static unsigned long lastToggleTime = 0;
    const unsigned long toggleDelay = 100; 

    bool currentPIRState = digitalRead(PIRPin);

    if (currentPIRState != lastPIRState) {
        if (currentPIRState == HIGH) {
            Serial.println("Motion detected!");
            client.publish(topicFromWokwi, "Motion detected!");
        } else {
            Serial.println("Motion stopped.");
            client.publish(topicFromWokwi, "Motion stopped.");
            digitalWrite(LED, LOW);
            noTone(buzzPin);
        }
        lastPIRState = currentPIRState;
    }

    if (currentPIRState == HIGH) {
        if (millis() - lastToggleTime > toggleDelay) {
            lastToggleTime = millis();
            if (digitalRead(LED) == LOW) {
                digitalWrite(LED, HIGH);
                tone(buzzPin, soundFrequency);
            } else {
                digitalWrite(LED, LOW);
                noTone(buzzPin);
            }
        }
    }
}
