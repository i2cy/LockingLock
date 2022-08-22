/*
 Publishing in the callback

  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"

  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the
  actual callback defined afterwards.
  This ensures the client reference in the MqttCmdCallback function
  is valid.

*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);

// Callback function header
void mqttCmdCallback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient MQTTClient(server, 1883, mqttCmdCallback, ethClient);

// Callback function
void mqttCmdCallback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.

  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  MQTTClient.publish("outTopic", p, length);
  // Free the memory
  free(p);
}

void setup()
{

  Ethernet.begin(mac, ip);
  if (MQTTClient.connect("arduinoClient")) {
    MQTTClient.publish("outTopic", "hello world");
    MQTTClient.subscribe("inTopic");
  }
}

void loop()
{
  MQTTClient.loop();
}
