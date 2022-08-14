/***------------------------------------***
 Project: LockingLock
 Filename: serial.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/


#include <Arduino.h>
#include "serial.h"
#include "modules/htsocket.h"
#include "modules/config.h"
#include "modules/mqtts.h"


#define serial_addr     0xcb                    // HTPack地址
#define cmd_fcode       0xcd                    // 指令功能字
#define MAX_CMD_LENGTH  32                      // 单个指令最大长度


extern bool FLAG_WIFI_FAILED;
extern char WIFI_SSID[64];
extern char WIFI_PSK[64];


// 静态文本
const char OK_CODE[] = "OK";


void initSerial() {
    Serial.begin(921600);
}

// 串口指令处理函数
void processSerialCmd(uint8_t *cmd_t) {
    char feed[64] = "";
    if (cmd_t[0] == 0x10) {  // WIFI配网-设置SSID
        writeSSIDConfig((char *)(cmd_t + 1));
        memmove(WIFI_SSID, (char *)(cmd_t + 1), sizeof(WIFI_SSID));
        readSSIDConfig(feed);
        Serial.print("SSID set: ");
        Serial.println(feed);
        sendHtpack((uint8_t *) &OK_CODE, serial_addr, cmd_fcode, sizeof(OK_CODE) - 1);
    }
    else if (cmd_t[0] == 0x11) {  // WIFI配网-设置密码
        writePSKConfig((char *)(cmd_t + 1));
        memmove(WIFI_PSK, (char *)(cmd_t + 1), sizeof(WIFI_PSK));
        readPSKConfig(feed);
        Serial.print("PSK set: ");
        Serial.println(feed);
        sendHtpack((uint8_t *) &OK_CODE, serial_addr, cmd_fcode, sizeof(OK_CODE) - 1);
    }
    else if (cmd_t[0] == 0x12) {  // WIFI配网-连接WIFI
        sendHtpack((uint8_t *) &OK_CODE, serial_addr, cmd_fcode, sizeof(OK_CODE) - 1);
        //FLAG_WIFI_FAILED = false;
        //reconnectWifiEventTask();
    }
}

// 串口事件循环任务
void serialEvent() {
    uint8_t cmd_t[MAX_CMD_LENGTH];
    uint8_t length;

    while (Serial.available()) {
        length = readHtpack(cmd_t, serial_addr, cmd_fcode);
        if (length > 0) {
            processSerialCmd(cmd_t);
        }
    }
}
