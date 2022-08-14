/***------------------------------------***
 Project: LockingLock
 Filename: led.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/13
***------------------------------------***/


#include "led.h"
#include <Arduino.h>


#define LED_FLASH_DURATION      10
#define LED_BLINK_ON_BURATION   3
#define LED_BLINK_BURATION      50


LEDManager_t g_LEDManager;


void ledSwitchOn(uint8_t pin) {
    digitalWrite(pin, 0);
}


void ledSwitchOff(uint8_t pin) {
    digitalWrite(pin, 1);
}


void ledSwitchToggle(uint8_t pin) {
    digitalWrite(pin, !(bool)digitalRead(pin));
}


void initLED() {
    g_LEDManager.flash_tick = 0;
    g_LEDManager.ledKnock = KNOCK_LED_OFF;
    g_LEDManager.ledNet = NET_LED_OFF;
    g_LEDManager.ledMotor = MOTOR_LED_OFF;

    pinMode(LED_NET_STATUS, OUTPUT);
    pinMode(LED_MOTOR_STATUS, OUTPUT);
    pinMode(LED_KNOCK_STATUS, OUTPUT);

    // 测试LED
    delay(200);
    ledSwitchOff(LED_NET_STATUS);
    delay(200);
    ledSwitchOff(LED_MOTOR_STATUS);
    delay(200);
    ledSwitchOff(LED_KNOCK_STATUS);
    delay(200);

    ledSwitchOn(LED_NET_STATUS);
    delay(200);
    ledSwitchToggle(LED_NET_STATUS);
    ledSwitchOn(LED_MOTOR_STATUS);
    delay(200);
    ledSwitchToggle(LED_MOTOR_STATUS);
    ledSwitchOn(LED_KNOCK_STATUS);
    delay(200);
    ledSwitchToggle(LED_KNOCK_STATUS);
}

// 20Hz 用于更新LED状态
void ledEventTask() {
    switch (g_LEDManager.ledMotor) {
        case MOTOR_LED_OFF:
            ledSwitchOff(LED_MOTOR_STATUS);
            break;

        case MOTOR_LED_ON:
            ledSwitchOn(LED_MOTOR_STATUS);
            break;

        case MOTOR_LED_BLINK:
            if ((g_LEDManager.flash_tick % LED_BLINK_BURATION) < LED_BLINK_ON_BURATION)
                ledSwitchOn(LED_MOTOR_STATUS);
            else
                ledSwitchOff(LED_MOTOR_STATUS);
            break;

        case MOTOR_LED_FLASH:
            if (!(g_LEDManager.flash_tick % LED_FLASH_DURATION))
                ledSwitchToggle(LED_MOTOR_STATUS);
            break;

        default:
            break;
    }

    switch (g_LEDManager.ledNet) {
        case NET_LED_OFF:
            ledSwitchOff(LED_NET_STATUS);
            break;

        case NET_LED_ON:
            ledSwitchOn(LED_NET_STATUS);
            break;

        case NET_LED_BLINK:
            if ((g_LEDManager.flash_tick % LED_BLINK_BURATION) < LED_BLINK_ON_BURATION)
                ledSwitchOn(LED_NET_STATUS);
            else
                ledSwitchOff(LED_NET_STATUS);
            break;

        case NET_LED_FLASH:
            if (!(g_LEDManager.flash_tick % LED_FLASH_DURATION))
                ledSwitchToggle(LED_NET_STATUS);
            break;

        default:
            break;
    }

    switch (g_LEDManager.ledKnock) {
        case KNOCK_LED_OFF:
            //ledSwitchOff(LED_KNOCK_STATUS);
            break;

        case KNOCK_LED_ON:
            ledSwitchOn(LED_KNOCK_STATUS);
            break;

        case KNOCK_LED_BLINK:
            if ((g_LEDManager.flash_tick % LED_BLINK_BURATION) < LED_BLINK_ON_BURATION)
                ledSwitchOn(LED_KNOCK_STATUS);
            else
                ledSwitchOff(LED_KNOCK_STATUS);
            break;

        case KNOCK_LED_FLASH:
            if (!(g_LEDManager.flash_tick % LED_FLASH_DURATION))
                ledSwitchToggle(LED_KNOCK_STATUS);
            break;

        default:
            break;
    }

    g_LEDManager.flash_tick = g_LEDManager.flash_tick < 30000 ? g_LEDManager.flash_tick + 1 : 0;
}