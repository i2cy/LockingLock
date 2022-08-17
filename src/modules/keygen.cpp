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


#define NTP_SERVER              "time.pool.aliyun.com"
#define GMT_OFFSET              -8*60*60
#define DAY_OFFSET              0
#define TIME_CALI_TICK          20*3600*24


extern bool FLAG_WIFI_FAILED;


uint32_t NPT_COUNTDOWN = 0;


void reconfigureTime() {
    if (FLAG_WIFI_FAILED) return;

    configTime(GMT_OFFSET, DAY_OFFSET, NTP_SERVER);
    NPT_COUNTDOWN = TIME_CALI_TICK;
}

// 20Hz
void timeCaliTask() {
    if (!NPT_COUNTDOWN) {
        reconfigureTime();
    }
}
