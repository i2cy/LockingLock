/***------------------------------------***
 Project: LockingLock
 Filename: led.h
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/13
***------------------------------------***/


#ifndef LOCKINGLOCK_LED_H
#define LOCKINGLOCK_LED_H

#endif //LOCKINGLOCK_LED_H


#include <Arduino.h>


#define LED_NET_STATUS              7
#define LED_MOTOR_STATUS            6
#define LED_KNOCK_STATUS            5

typedef enum {
    NET_LED_OFF = 0,
    NET_LED_ON,
    NET_LED_BLINK,
    NET_LED_FLASH
} em_LEDNetStatus_t;

typedef enum {
    MOTOR_LED_OFF = 0,
    MOTOR_LED_ON,
    MOTOR_LED_BLINK,
    MOTOR_LED_FLASH
} em_LEDMotorStatus_t;

typedef enum {
    KNOCK_LED_OFF = 0,
    KNOCK_LED_ON,
    KNOCK_LED_BLINK,
    KNOCK_LED_FLASH
} em_LEDKnockStatus_t;

typedef struct {
    uint16_t flash_tick;

    em_LEDKnockStatus_t ledKnock;
    em_LEDMotorStatus_t ledMotor;
    em_LEDNetStatus_t ledNet;
} LEDManager_t;


// 初始化LED
void initLED();
void ledEventTask();