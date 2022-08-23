/***------------------------------------***
 Project: LockingLock
 Filename: keygen.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/14
***------------------------------------***/


#include <Arduino.h>
#include "keygen.h"
#include "time.h"
#include "mqtts.h"
#include "htsocket.h"
#include "Dynkey16.h"
#include "WiFi.h"


#define NTP_SERVER              "asia.pool.ntp.org"
#define GMT_OFFSET              8*60*60
#define DAY_OFFSET              0
#define TIME_CALI_TICK          20*3600*24
#define DK_PSK                  "testtest123"
#define DK_PSK_LENGTH           11

#define DEBUG_ADDR              0xcb
#define DEBUG_FCODE             0x02


extern bool FLAG_WIFI_FAILED;


int32_t TIME_OFFSET_SEC = 0;
Key_t DPSK;
Dynkey16 DKgen = nullptr;


void initKeygen() {
    memcpy(DPSK.value, DK_PSK, DK_PSK_LENGTH);
    DPSK.len = DK_PSK_LENGTH;

    DKgen = Dynkey16(&DPSK);
}


// 5Hz
void dynkeyDebugTask() {
    //uint8_t buf[16 + 4];
    Key_t test_key;
    DKgen.keygen(&test_key);
    /*memcpy(buf, test_key.value, test_key.len);
    buf[16] = test_key.timestamp >> 24;
    buf[17] = test_key.timestamp >> 16;
    buf[18] = test_key.timestamp >> 8;
    buf[19] = test_key.timestamp;
    sendHtpack(buf, DEBUG_ADDR, DEBUG_FCODE, 16 + 4);*/
}


void setDynkey16TimeOffset(int32_t offset) {
    DKgen.setTimeOffset(offset);
}
