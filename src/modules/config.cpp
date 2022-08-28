//
// Created by i2cy on 2022/7/13.
//

#define CONFIG              "config"

#define WIFI_SSID_ADDR      "WIFI_SSID"
#define WIFI_PSK_ADDR       "WIFI_PSK"
#define MOTOR_OFFSET        "MOTOR_OFFSET"

#include "config.h"


Preferences pref;


// 读取SSID
void readSSIDConfig(char *dst) {
    pref.begin(CONFIG);
    if (pref.isKey(WIFI_SSID_ADDR))
        pref.getString(WIFI_SSID_ADDR, dst, 64);
    pref.end();
}


// 写入SSID
void writeSSIDConfig(char *ssid) {
    Serial.print("setting SSID: ");
    Serial.println(ssid);
    pref.begin(CONFIG);
    pref.putString(WIFI_SSID_ADDR, ssid);
    pref.end();
}


// 读取PSK
void readPSKConfig(char *dst) {
    pref.begin(CONFIG);
    if (pref.isKey(WIFI_PSK_ADDR))
        pref.getString(WIFI_PSK_ADDR, dst, 64);
    pref.end();
}


// 写入PSK
void writePSKConfig(char *psk) {
    Serial.print("setting PSK: ");
    Serial.println(psk);
    pref.begin(CONFIG);
    pref.putString(WIFI_PSK_ADDR, psk);
    pref.end();
}


// 读取电机行程偏移量
uint32_t readMotorOffsetConfig() {
    uint32_t ret = 0;
    pref.begin(CONFIG);
    if (pref.isKey(MOTOR_OFFSET))
        ret = pref.getULong(MOTOR_OFFSET);
    pref.end();

    return ret;
}


// 写入电机行程偏移量
void writeMotorOffsetConfig(uint32_t value) {
    Serial.print("setting motor offset: ");
    Serial.println(value);
    pref.begin(CONFIG);
    pref.putULong(MOTOR_OFFSET, value);
    pref.end();
}