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
#include "config.h"
#include "led.h"
#include "motor.h"
#include "keygen.h"


const char *MQTT_SERVER_PTR = "i2cy.tech";      //MQTTS服务器地址
const int MQTT_PORT = 8883;                     //端口号

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

#define MQTT_DEVID          "esp_device001"         // 设备ID

#define MQTT_CMD_TOPIC      "esp32test/request/#"   // 订阅指令主题
#define MQTT_FB_TOPIC       "esp32test/data"        // 上发主题

#define MQTT_HEARTBEAT_SEC  5                       // 心跳间隔

#define CMD_ID_HEARTBEAT    0x00                    // 心跳
#define CMD_ID_REQTIMECALI  0x01                    // 请求校准时间
#define CMD_ID_REQCONFIG    0x02                    // 请求接收配置
#define CMD_ID_SENDCALI     0x30                    // 发送校准数据
#define CMD_ID_REQOK        0xff                    // OK

#define CMD_ID_TIMECALI     0x11                    // 校准时间
#define CMD_ID_CONFIG       0x12                    // 设置配置
#define CMD_ID_OFFSETCALI   0x13                    // 校准门锁行程
#define CMD_ID_OPENGATE     0x20                    // 通知开门
#define CMD_ID_RING         0x21                    // 发出响声
#define CMD_ID_OK           0xee                    // OK

#define CMD_TIMEOUT         (10 * 200)              // 指令获取反馈超时计数


extern LEDManager_t g_LEDManager;
extern MotorManager_t g_MotorManager;
extern int32_t TIME_OFFSET_SEC;


// 创建TLS加密的WIFI客户端
WiFiClientSecure WifiClient;

// 初始化PubSub客户端
PubSubClient MQTTClient(WifiClient);

// 标志位
bool FLAG_WIFI_FAILED = false;
uint8_t OK_FEEDBACK = 0;

// WIFI信息
char WIFI_SSID[64] = "";
char WIFI_PSK[64] = "";

// MQTT鉴权信息
char MQTT_USER[64] = "";
char MQTT_PWD[64] = "";

// 定时器,用来循环上传数据
Ticker HeartBeat_TIM;


void mqttSendCommand(const char *topic, uint8_t cmd_id, const uint8_t *payload, uint16_t length) {
    static bool global_lock = false;
    static uint8_t flow_cnt = 0;
    bool sent = false;
    uint8_t buf[length + 5], checksum = 0;
    uint16_t countdown = CMD_TIMEOUT;

    buf[0] = flow_cnt;
    buf[1] = cmd_id;
    buf[2] = length >> 8;
    buf[3] = length;
    memcpy(buf + 4, payload, length);
    buf[length + 4] = 0;

    for (uint8_t &i: buf) checksum += i;
    buf[length + 4] = checksum;

    while (global_lock) {
        vTaskDelay(1);
    }

    global_lock = true;
    flow_cnt++;

    if (cmd_id > 0 && cmd_id < 0xff) OK_FEEDBACK++;

    while (!sent || OK_FEEDBACK) {
        if (countdown) {
            countdown--;
        }
        else {
            sent = false;
            countdown = CMD_TIMEOUT;
        }

        if (!sent) {
            MQTTClient.publish(topic, buf, length + 5);
            sent = true;
        }
        vTaskDelay(5);
    }

    global_lock = false;
}


// 收到主题下发的回调, 注意这个回调要实现三个形参 1:topic 主题， 2: payload: 传递过来的信息， 3: length: 长度
void mqttCmdCallback(char *topic, byte *payload, unsigned int length) {
    auto *header = (MqttCmdHeader_t *) payload;
    uint8_t checksum;

    Serial.print("Received commandID: ");
    Serial.print(header->cmd_id);
    Serial.print("\n");

    checksum = 0;

    for (uint32_t i = 0; i < (length - 1); i++) checksum += payload[i];

    Serial.print("Checksum result: ");
    Serial.print(checksum);
    Serial.print("\n");

    if (checksum != payload[length - 1]) return;

    if (header->cmd_id == CMD_ID_TIMECALI) {                    // 校准时间
        time_t now;
        time(&now);

        TIME_OFFSET_SEC = *((int32_t *) (payload + 4)) - now;
        setDynkey16TimeOffset(TIME_OFFSET_SEC);
        sendOK();
    }
    else if (header->cmd_id == CMD_ID_CONFIG) {                 // 设置配置
        g_MotorManager.total_steps = *((uint32_t *) (payload + 4));
        sendOK();
    }
    else if (header->cmd_id == CMD_ID_OFFSETCALI) {             // 校准门锁行程
        setMotorCaliOffset();
        sendOK();
    }
    else if (header->cmd_id == CMD_ID_OPENGATE) {               // 通知开门
        setMotorCmdUnlock();
        sendOK();
    }
    else if (header->cmd_id == CMD_ID_RING) {                   // 发出响声
        ringMotor();
        sendOK();
    }
    else if (header->cmd_id == CMD_ID_OK) {                     // OK
        if (OK_FEEDBACK) OK_FEEDBACK--;
    }
}


// 发送心跳
void sendHeartbeat() {
    time_t ts;

    if (MQTTClient.connected()) {
        time(&ts);
        ts += TIME_OFFSET_SEC;
        mqttSendCommand(MQTT_FB_TOPIC, CMD_ID_HEARTBEAT, (uint8_t *) &ts, 4);
        //Serial.print("heartbeat sent\n");
    }
}


// 发送校准数据
void sendCaliOffset(uint32_t offset) {
    if (MQTTClient.connected()) {
        mqttSendCommand(MQTT_FB_TOPIC, CMD_ID_SENDCALI, (uint8_t *) &offset, 4);
    }
}


// 发送请求接收配置
void sendConfigRequest() {
    time_t ts;

    if (MQTTClient.connected()) {
        time(&ts);
        ts += TIME_OFFSET_SEC;
        mqttSendCommand(MQTT_FB_TOPIC, CMD_ID_REQCONFIG, (uint8_t *) &ts, 4);
    }
}


// 发送请求校准时间
void sendTimeCaliRequest() {
    time_t ts;

    if (MQTTClient.connected()) {
        time(&ts);
        ts += TIME_OFFSET_SEC;
        mqttSendCommand(MQTT_FB_TOPIC, CMD_ID_REQTIMECALI, (uint8_t *) &ts, 4);
    }
}


// 发送OK
void sendOK() {
    time_t ts;

    if (MQTTClient.connected()) {
        time(&ts);
        ts += TIME_OFFSET_SEC;
        mqttSendCommand(MQTT_FB_TOPIC, CMD_ID_REQOK, (uint8_t *) &ts, 4);
    }
}


// MQTT连接函数
bool setupMQTT() {
    bool ret;
    // 连接到服务器
    MQTTClient.setServer(MQTT_SERVER_PTR, MQTT_PORT);
    readMqttUsernameConfig(MQTT_USER);
    readMqttPwdConfig(MQTT_PWD);

    ret = MQTTClient.connect(MQTT_DEVID, MQTT_USER, MQTT_PWD);
    if (MQTTClient.connected()) {
        MQTTClient.subscribe(MQTT_CMD_TOPIC); //订阅命令下发主题
    }
    return ret;
}


// 10Hz WIFI连接任务
void reconnectWifiEventTask() {
    static uint8_t countdown = 0;

    if (WiFi.isConnected()) {
        FLAG_WIFI_FAILED = false;
        countdown = 0;
        return;
    }

    if (countdown) {
        countdown--;
        return;
    }
    else {
        readSSIDConfig(WIFI_SSID);
        readPSKConfig(WIFI_PSK);

        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PSK);
        countdown = 100;
    }
}


// 10Hz 重连任务
void clientReconnectEventTask() {
    static uint8_t countdown = 0;

    if (MQTTClient.connected() || FLAG_WIFI_FAILED) {
        countdown = 0;
        return;
    }

    if (countdown) {
        countdown--;
        return;
    }

    if (!setupMQTT()) {
        countdown = 50;
    }
}


void initMqtt() {
    // 连接WIFI
    reconnectWifiEventTask();
    // 设置服务器证书
    WifiClient.setCACert(MQTT_CA_CERT);
    // 加载连接对象到MQTT客户端
    MQTTClient.setClient(WifiClient);
    // 连接MQTT
    setupMQTT();
    // 设置MQTT客户端消息回调函数
    MQTTClient.setCallback(mqttCmdCallback);

    // 设置定时发送
    HeartBeat_TIM.attach(MQTT_HEARTBEAT_SEC, sendHeartbeat);
}


// 非实时
void MqttTask() {
    if (!WiFi.isConnected()) {
        FLAG_WIFI_FAILED = true;
        g_LEDManager.ledNet = NET_LED_FLASH;
    }
    else if (!MQTTClient.connected()) {
        g_LEDManager.ledNet = NET_LED_BLINK;
    }
    else {
        g_LEDManager.ledNet = NET_LED_ON;
    }

    MQTTClient.loop(); //客户端循环检测
}
