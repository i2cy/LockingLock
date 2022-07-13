#include <Arduino.h>
#include <cstring>
#include "WiFi.h"
#include "PubSubClient.h"
#include "Ticker.h"
#include "WiFiClientSecure.h"
#include "modules/htsocket.h"
#include "modules/config.h"


const char *mqtt_server = "auritek.top";    //MQTTS服务器地址
const int port = 8883;                      //端口号

static const char mqtt_ca_crt[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICzTCCAbUCFGAt2MC01jfCJl4PXQErw1FrFxM1MA0GCSqGSIb3DQEBCwUAMCMx
CzAJBgNVBAYTAkNOMRQwEgYDVQQDDAthdXJpdGVrLnRvcDAeFw0yMjA3MTAwNTUw
NTlaFw0yMjEwMTgwNTUwNTlaMCMxCzAJBgNVBAYTAkNOMRQwEgYDVQQDDAthdXJp
dGVrLnRvcDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALLGLavkNsI1
h52RhpHUKk6JqaUXRN928Tlud2VrzX79XDGzE2x8oAnSDr6bbVOCsMujYDvT5Cly
VqZEBUz+lzWHuqaGkB5g0j/ikrfndWqQJ8++I/1ejwWQ7D7VbIcYYlzw+h+VSP5Y
EKcLGfukAA8U6r06dPvoxyDjfGTnkYP4FJVyki/IFB6pb5Wt+8mG3MBRktPDVimp
ZzG0e2pj7DuJMKKOXVqJDj5QfbXzx0yi5GWPDbhkyfpuFmOue1EcNiy2kX33uLjJ
IhxKma/m7G3INntECu4XLi6feDpvhkAYU4wGAZ/wFa5gnuIRwT+3SI0X+tbwCsGo
V0I1m0pAMJkCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEASkBnXopkFhUWDFFbkdVV
wul9qebpLLLrL4uBNFfksnDyMddSQsiB3dTg5L4RUKdtefGwsAnbH8R23Ce88gRU
PPUk2OATzAJruFIVTRuUwjY05Zg+Cqu1jX5Ngpj6WLQuol22z58Ooc8VQU5QpRRP
tzEoVm+fbvMDmXivG/l6o1DA22PntOEoEuz3l8L0j0CvHRmDmUd6vMTiyoVZoTJL
fvGmM1rulEWQXs6fD6nbD/kgUmusWDqpB4xu75OLVvZP5fKfvmMmUrh6qjl3uFAr
Jp6KcJpvXof/hoSlrhzar1C6wPW4hvELHwtTa8YudSoKVCo0sqnOwSubVr0buttC
Ew==
-----END CERTIFICATE-----
)EOF";

#define mqtt_devid      "esp_device001"         // 设备ID
#define serial_addr     0xcb                    // HTPack地址
#define cmd_fcode       0xcd                    // 指令功能字

#define mqtt_pubid      "icy"                   // MQTT用户名
#define mqtt_password   "__C4o0d9y6#"           // MQTT密码

#define mqtt_cmd_topic  "esp32test/request/#"   // 订阅指令主题
#define mqtt_feed_topic "esp32test/data"        // 反馈主题

#define cmd_length      32                      // 单个指令最大长度


// 创建TLS加密的WIFI客户端
WiFiClientSecure wifiClient;

// 初始化PubSub客户端
PubSubClient client(wifiClient);

// 发送信息缓冲区
char msgJson[75];

// 信息模板
char dataTemplate[] = "{\"id\":123,\"dp\":{\"temp\":[{\"v\":%.2f}],\"hull\":[{\"v\":%.2f}]}}";

// 标志位
bool flag_ssid_failed = false;

// 静态文本
const char OK_CODE[] = "OK";

// 定时器,用来循环上传数据
Ticker tim1;


// 收到主题下发的回调, 注意这个回调要实现三个形参 1:topic 主题， 2: payload: 传递过来的信息， 3: length: 长度
void callback(char *topic, byte *payload, unsigned int length) {
    Serial.println("message rev:");
    Serial.println(topic);
    for (size_t i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
}

// 向主题发送模拟的温湿度数据
void sendTempAndHumi() {
    if (client.connected()) {
        //将模拟温湿度数据套入dataTemplate模板中, 生成的字符串传给msgJson
        snprintf(msgJson, 75, dataTemplate, 22.5, 35.6);
        Serial.print("public the data:");
        Serial.println(msgJson);

        //发送数据到主题
        client.publish(mqtt_feed_topic, (uint8_t *) msgJson, strlen(msgJson));
    }
}

// MQTT连接函数
bool setupMQTT() {
    bool ret;

    // 连接到服务器
    client.setServer(mqtt_server, port);
    ret = client.connect(mqtt_devid, mqtt_pubid, mqtt_password);
    if (client.connected()) {
        Serial.println("AuriNet is connected!");
        client.subscribe(mqtt_cmd_topic); //订阅命令下发主题
    }
    return ret;
}

// WIFI连接函数
void setupWifi() {
    uint8_t i = 0;
    char ssid[64] = "";
    char psk[64] = "";

    if (flag_ssid_failed) return;

    delay(10);
    Serial.print("Connecting to WiFi via SSID: \"");

    readSSIDConfig(ssid);
    readPSKConfig(psk);

    Serial.print(ssid);
    Serial.print("\"");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, psk);

    while (!WiFi.isConnected()) {
        if (i > 10) {
            flag_ssid_failed = true;
            Serial.println("Failed");
            return;
        } else {
            Serial.print(".");
        }
        delay(1000);
        i++;
    }
    Serial.println("OK");
    Serial.println("Wifi connected");
}

// 重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect() {
    if (flag_ssid_failed) return;

    Serial.println("reconnect MQTT...");
    if (setupMQTT()) {
    } else {
        Serial.println("failed");
        Serial.println(client.state());
        Serial.println("try again in 5 sec");
        delay(5000);
    }
}


void processSerialCmd(uint8_t *cmd_t) {
    char feed[64] = "";
    if (cmd_t[0] == 0x10) {  // WIFI配网-设置SSID
        writeSSIDConfig((char *)(cmd_t + 1));
        readSSIDConfig(feed);
        Serial.print("SSID set: ");
        Serial.println(feed);
        sendHtpack((uint8_t *) &OK_CODE, serial_addr, cmd_fcode, sizeof(OK_CODE) - 1);
    } else if (cmd_t[0] == 0x11) {  // WIFI配网-设置密码
        writePSKConfig((char *)(cmd_t + 1));
        readPSKConfig(feed);
        Serial.print("PSK set: ");
        Serial.println(feed);
        sendHtpack((uint8_t *) &OK_CODE, serial_addr, cmd_fcode, sizeof(OK_CODE) - 1);
    } else if (cmd_t[0] == 0x12) {  // WIFI配网-连接WIFI
        sendHtpack((uint8_t *) &OK_CODE, serial_addr, cmd_fcode, sizeof(OK_CODE) - 1);
        flag_ssid_failed = false;
        setupWifi();
    }
}


void setup() {
    // 初始化串口
    Serial.begin(115200);
    // 连接WIFI
    setupWifi();
    // 设置服务器证书
    wifiClient.setCACert(mqtt_ca_crt);
    // 加载连接对象到MQTT客户端
    client.setClient(wifiClient);
    // 连接MQTT
    setupMQTT();
    // 设置MQTT客户端消息回调函数
    client.setCallback(callback);

    // 设置定时发送
    tim1.attach(5, sendTempAndHumi);
}


// 串口事件循环任务
void serialEvent() {
    uint8_t cmd_t[cmd_length];
    uint8_t length;

    while (Serial.available()) {
        length = readHtpack(cmd_t, serial_addr, cmd_fcode);
        if (length > 0) {
            processSerialCmd(cmd_t);
        }
    }
}


void loop() {
    if (!WiFi.isConnected())  // 检查WIFI状态，若未连接则尝试连接
    {
        setupWifi();
    }
    if (!client.connected())  // 检查MQTT客户端状态，若未连接则尝试连接
    {
        clientReconnect();
        delay(100);
    }
    client.loop(); //客户端循环检测
    serialEvent();
}
