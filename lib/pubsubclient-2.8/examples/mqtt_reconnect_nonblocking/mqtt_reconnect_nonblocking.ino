/*
 Reconnecting MQTT example - non-blocking

 This sketch demonstrates how to keep the client connected
 using a non-blocking reconnect function. If the client loses
 its connection, it attempts to reconnect every 5 seconds
 without blocking the main loop.

*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your hardware/network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);

void mqttCmdCallback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

EthernetClient ethClient;
PubSubClient MQTTClient(ethClient);

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (MQTTClient.connect("arduinoClient")) {
    // Once connected, publish an announcement...
    MQTTClient.publish("outTopic", "hello world");
    // ... and resubscribe
    MQTTClient.subscribe("inTopic");
  }
  return MQTTClient.connected();
}

void setup()
{
  MQTTClient.setServer(server, 1883);
    MQTTClient.setCallback(mqttCmdCallback);

  Ethernet.begin(mac, ip);
  delay(1500);
  lastReconnectAttempt = 0;
}


void loop()
{
  if (!MQTTClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected

    MQTTClient.loop();
  }

}
