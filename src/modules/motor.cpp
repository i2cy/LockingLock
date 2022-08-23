#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include "motor.h"
#include "led.h"
#include "mpu6050.h"
#include "mqtts.h"


#define MOTOR_A                     11
#define MOTOR_B                     9
#define MOTOR_C                     8
#define MOTOR_D                     10

#define TRIGGER                     14

#define MAX_IDLE_TIME               6000

#define UNLOCK_HOLD_TIME            600
#define UNLOCK_BY_CMD_TIMEOUT       4000
#define UNLOCK_BY_CMD_KNOCK_COUNT   3

#define CALI_MOTOR_HOLD_TIME        2000


extern LEDManager_t g_LEDManager;
extern VibeManager_t g_VibeManager;


Stepper MOTOR = Stepper(64,
                        MOTOR_A, MOTOR_B,
                        MOTOR_C, MOTOR_D);

MotorManager_t g_MotorManager;

bool TRIGGERED = false;


void initMotor()
{
    // initialize step motor
    pinMode(MOTOR_A, OUTPUT);
    pinMode(MOTOR_B, OUTPUT);
    pinMode(MOTOR_C, OUTPUT);
    pinMode(MOTOR_D, OUTPUT);

    // initialize trigger
    pinMode(TRIGGER, INPUT_PULLDOWN);

    g_MotorManager.current_steps = 1000;
    g_MotorManager.total_steps = 1000;
    g_MotorManager.status = IDLE;
    g_MotorManager.countdown = 0;
    g_MotorManager.sequence = LOCKED;
    g_MotorManager.cali_enabled = false;

    ringMotor();
    setMotorStop();
}


void setMotorStop() {
    digitalWrite(MOTOR_A, 0);
    digitalWrite(MOTOR_B, 0);
    digitalWrite(MOTOR_C, 0);
    digitalWrite(MOTOR_D, 0);
}


void setMotorPskUnlock() {
    g_MotorManager.sequence = UNLOCK_BY_PSK;
}


void setMotorCmdUnlock() {
    g_MotorManager.countdown = UNLOCK_BY_CMD_TIMEOUT;
    g_MotorManager.sequence = WAITING_BY_CMD;
}


void setMotorCaliOffset() {
    g_MotorManager.sequence = CALI_BY_CMD;
}


void retractMotor() {
    if (TRIGGERED) {
        if (g_MotorManager.cali_enabled && g_MotorManager.current_steps != 0) {
            g_MotorManager.total_steps = -g_MotorManager.current_steps;
            sendCaliOffset(g_MotorManager.total_steps);
        }

        g_MotorManager.current_steps = 0;
        g_MotorManager.status = IDLE;
    }
    else {
        MOTOR.stepOne(true);
        g_MotorManager.current_steps--;
    }
}


void releaseMotor() {
    if (g_MotorManager.current_steps < g_MotorManager.total_steps) {
        MOTOR.stepOne(false);
        g_MotorManager.current_steps++;
    }
    else {
        g_MotorManager.status = STOP;
    }
}


void ringMotorOnOpen() {
    for (int i = 0; i<100; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1800);
        MOTOR.stepOne(false);
        delayMicroseconds(1500);
    }
    delay(500);
    for (int i = 0; i<100; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1500);
        MOTOR.stepOne(false);
        delayMicroseconds(1800);
    }
    delay(500);
    for (int i = 0; i<100; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1800);
        MOTOR.stepOne(false);
        delayMicroseconds(1500);
    }
    delay(500);
    for (int i = 0; i<100; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1500);
        MOTOR.stepOne(false);
        delayMicroseconds(1800);
    }
    delay(500);
    for (int i = 0; i<200; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1300);
        MOTOR.stepOne(false);
        delayMicroseconds(1300);
    }
}


void ringMotor() {
    for (int i = 0; i<100; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1300);
        MOTOR.stepOne(false);
        delayMicroseconds(1300);
    }
    for (int i = 0; i<100; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1500);
        MOTOR.stepOne(false);
        delayMicroseconds(1500);
    }
    for (int i = 0; i<100; i++) {
        MOTOR.stepOne(true);
        delayMicroseconds(1800);
        MOTOR.stepOne(false);
        delayMicroseconds(1800);
    }
}


void triggerTask() {
    TRIGGERED = digitalRead(TRIGGER);
    if (TRIGGERED) {
        g_LEDManager.ledMotor = MOTOR_LED_FLASH;
    }
}


void motorSequenceTask() {
    static uint32_t last_countdown = 0;

    switch (g_MotorManager.sequence) {
        case LOCKED:
            break;

        case LOCKING:
            if (g_MotorManager.current_steps == g_MotorManager.total_steps) {
                g_MotorManager.sequence = LOCKED;
            }
            else {
                g_MotorManager.status = RELEASE;
            }
            break;

        case WAITING_BY_CMD:
            if (g_MotorManager.countdown) {
                if (g_VibeManager.sequence[g_VibeManager.current_sequence] >= UNLOCK_BY_CMD_KNOCK_COUNT) {
                        ringMotorOnOpen();
                        g_MotorManager.sequence = UNLOCK_BY_CMD;
                        last_countdown = 0;

                }
                g_MotorManager.countdown--;
            }
            else {
                g_MotorManager.sequence = LOCKING;
            }
            break;

        case UNLOCK_BY_CMD:
            if (TRIGGERED) {
                g_MotorManager.countdown = UNLOCK_HOLD_TIME;
                g_MotorManager.sequence = HOLD_BY_CMD;
            }
            else {
                g_MotorManager.status = RETRACT;
            }
            break;

        case HOLD_BY_CMD:
            if (g_MotorManager.countdown) {
                g_MotorManager.countdown--;
            }
            else {
                g_MotorManager.sequence = LOCKING;
            }
            break;

        case CALI_BY_CMD:
            if (TRIGGERED) {
                g_MotorManager.countdown = CALI_MOTOR_HOLD_TIME;
                g_MotorManager.sequence = HOLD_CALI_BY_CMD;
                g_MotorManager.status = STOP;
            }
            else {
                g_MotorManager.status = RETRACT;
            }
            break;

        case HOLD_CALI_BY_CMD:
            if (g_MotorManager.countdown) {
                g_MotorManager.countdown--;
            }
            else {
                g_MotorManager.sequence = CALI_FINISH;
                g_MotorManager.cali_enabled = true;
            }
            break;

        case CALI_FINISH:
            if (TRIGGERED) {
                g_MotorManager.sequence = LOCKING;
                g_MotorManager.cali_enabled = false;
            }
            else {
                g_MotorManager.status = RETRACT;
            }
            break;

        default:
            break;
    }
}


void motorTask() {
    static uint16_t idle_countdown = MAX_IDLE_TIME;
    switch (g_MotorManager.status) {
        case STOP:
            g_LEDManager.ledMotor = MOTOR_LED_OFF;
            setMotorStop();
            break;

        case IDLE:
            if (idle_countdown) {
                idle_countdown--;
                g_LEDManager.ledMotor = MOTOR_LED_ON;
            }
            else {
                idle_countdown = MAX_IDLE_TIME;
                g_MotorManager.status = STOP;
            }
            break;

        case RETRACT:
            g_LEDManager.ledMotor = MOTOR_LED_BLINK;
            retractMotor();
            break;

        case RELEASE:
            g_LEDManager.ledMotor = MOTOR_LED_BLINK;
            releaseMotor();
            break;

        default:
            break;
    }
}