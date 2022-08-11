//
// Created by Adminadorstr on 2022/8/11.
//

#include "mqtts.h"
#include <Arduino.h>
#include <cstring>
#include "WiFi.h"
#include "PubSubClient.h"
#include "Ticker.h"
#include "WiFiClientSecure.h"
#include "modules/config.h"


const char *MQTT_SERVER_PTR = "i2cy.tech";      //MQTTS服务器地址
const int MQTT_PORT = 8883;                      //端口号

static const char MQTT_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICyzCCAbMCFDpQeOKsa3bRNsq54wNG5C6qhSoxMA0GCSqGSIb3DQEBCwUAMCEx
CzAJBgNVBAYTAkNOMRIwEAYDVQQDDAlpMmN5LnRlY2gwIBcNMjIwNzA4MTcwMzUw
WhgPMjEyMjA2MTQxNzAzNTBaMCExCzAJBgNVBAYTAkNOMRIwEAYDVQQDDAlpMmN5
LnRlY2gwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC7W+0i/MDfJVii
avaaaCKH1gD4XyMDShuhnh7RbfXnBGW0p93Nu5MznyWiXCyyv0LpX6MYn+1UA0qE
K1Z3puZR8TXhEApI2vEHB9++xqsKeSOj039tGQXHRGYM/LQbncEVag2dRbbb5FnU
2ZhXwLxoB4WnIFqngZVyUVePEDw5R4GZS/+pmz2M9EGBRKZ3ERB1vgrhev2zS2RP
8XzXU10MYWJiT+ROi96hYZ25Fbpdk3aRMptXd6I/IzNEzLlqzBshEJw9yGagm8Qu
IJJAoZd8JVrAgr/OIXEmZ6D0aTeHd9qz+9gFQ860muvW1ElcCRh7ZllhrA0MFscr
xqrB3qoHAgMBAAEwDQYJKoZIhvcNAQELBQADggEBANaEjNcxdQ9/Ku7YilvTYNIS
NWUI/dC+O2r21z3uz4PlqSSY8RSo4PAUoKFrcxjQ7QWoIc00q1FTbrdKJV4gyQ8G
XWe5zcHuiCrr7E0c4CEhdwPAAJQOVzS6E1jLKuxKV2CcgfvJGju9i+Ib+l7CzO+m
RXAhnHOs8EMH7PsKV5a+bbtJ3ULITGxqSl7h3LNIyuKm6mNi/f6oHsZ0J+7jLa0Z
/IQvHFOIv0duh7QQv0DfaYVnvNPcSQnysigI8qpQmEjfo4vmEXi8DEWUpC3kvKU9
VDRIHMERipibUWtLKgEEhDtb88x14DsrZTsp3mb0+nkt1lDBR9rUpLVDyI+CiVE=
-----END CERTIFICATE-----
)EOF";

#define MQTT_DEVID      "esp_device001"         // 设备ID


#define MQTT_PUBID      "icy"                   // MQTT用户名
#define MQTT_PASSWORD   "__C4o0d9y6#"           // MQTT密码

#define MQTT_CMD_TOPIC  "esp32test/request/#"   // 订阅指令主题
#define MQTT_FB_TOPIC   "esp32test/data"        // 反馈主题


// 创建TLS加密的WIFI客户端
WiFiClientSecure WifiClient;

// 初始化PubSub客户端
PubSubClient MQTTClient(WifiClient);

// 发送信息缓冲区
char msgJson[75];

// 信息模板
char dataTemplate[] = "{\"id\":123,\"dp\":{\"temp\":[{\"v\":%.2f}],\"hull\":[{\"v\":%.2f}]}}";

// 标志位
bool FLAG_SSID_FAILED = false;

// WIFI信息
char WIFI_SSID[64] = "";
char WIFI_PSK[64] = "";

// 定时器,用来循环上传数据
Ticker tim1;


// 收到主题下发的回调, 注意这个回调要实现三个形参 1:topic 主题， 2: payload: 传递过来的信息， 3: length: 长度
void MqttCmdCallback(char *topic, byte *payload, unsigned int length) {
    Serial.println("message rev:");
    Serial.println(topic);
    for (size_t i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
}

// 向主题发送模拟的温湿度数据
void sendTempAndHumi() {
    if (MQTTClient.connected()) {
        //将模拟温湿度数据套入dataTemplate模板中, 生成的字符串传给msgJson
        snprintf(msgJson, 75, dataTemplate, 22.5, 35.6);
        //Serial.print("public the data:");
        //Serial.println(msgJson);

        //发送数据到主题
        MQTTClient.publish(MQTT_FB_TOPIC, (uint8_t *) msgJson, strlen(msgJson));
    }
}

// MQTT连接函数
bool setupMQTT() {
    bool ret;

    // 连接到服务器
    MQTTClient.setServer(MQTT_SERVER_PTR, MQTT_PORT);
    ret = MQTTClient.connect(MQTT_DEVID, MQTT_PUBID, MQTT_PASSWORD);
    if (MQTTClient.connected()) {
        Serial.println("I2Net is connected!");
        MQTTClient.subscribe(MQTT_CMD_TOPIC); //订阅命令下发主题
    }
    return ret;
}

// WIFI连接函数
void setupWifi() {
    static uint8_t i = 0;
    //char WIFI_SSID[64] = "";
    //char WIFI_PSK[64] = "";

    //if (FLAG_SSID_FAILED) return;

    delay(10);
    Serial.print("Connecting to WiFi via SSID: \"");

    readSSIDConfig(WIFI_SSID);
    readPSKConfig(WIFI_PSK);

    Serial.print(WIFI_SSID);
    Serial.print("\"");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    if (!WiFi.isConnected()) {
        if (i > 10) {
            FLAG_SSID_FAILED = true;
            Serial.println("Failed");
            i = 0;
            return;
        } else {
            Serial.print(".");
        }
        delay(1000);
        i++;
    }
    else {
        FLAG_SSID_FAILED = false;
        Serial.println("OK");
        Serial.println("Wifi connected");
    }
}

// 重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect() {
    if (FLAG_SSID_FAILED) return;

    Serial.println("reconnect MQTT...");
    if (setupMQTT()) {
    }
    else {
        Serial.println("failed");
        Serial.println(MQTTClient.state());
        Serial.println("try again in 5 sec");
        delay(5000);
    }
}

void initMqtt() {
    // 连接WIFI
    setupWifi();
    // 设置服务器证书
    WifiClient.setCACert(MQTT_CA_CERT);
    // 加载连接对象到MQTT客户端
    MQTTClient.setClient(WifiClient);
    // 连接MQTT
    setupMQTT();
    // 设置MQTT客户端消息回调函数
    MQTTClient.setCallback(MqttCmdCallback);

    // 设置定时发送
    tim1.attach(5, sendTempAndHumi);
}


void MqttTask() {
    if (!WiFi.isConnected()) { // 检查WIFI状态，若未连接则尝试连接
        setupWifi();
    }
    else if (!MQTTClient.connected()) { // 检查MQTT客户端状态，若未连接则尝试连接
        clientReconnect();
        delay(100);
    }

    MQTTClient.loop(); //客户端循环检测

    //serialEvent();
}
