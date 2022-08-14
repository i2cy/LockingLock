#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include "modules/motor.h"
#include "led.h"


#define MOTOR_A                     11
#define MOTOR_B                     9
#define MOTOR_C                     8
#define MOTOR_D                     10

#define TRIGGER                     14
#define CALI_MOTOR_THRESHOLD        20

#define MAX_IDLE_TIME               5000


extern LEDManager_t g_LEDManager;


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
    g_MotorManager.status = RETRACT;

    MOTOR.setSpeed(50);

    ringMotor();
    setMotorStop();
}


void setMotorStop() {
    digitalWrite(MOTOR_A, 0);
    digitalWrite(MOTOR_B, 0);
    digitalWrite(MOTOR_C, 0);
    digitalWrite(MOTOR_D, 0);
}


void retractMotor() {
    if (TRIGGERED) {
        if (g_MotorManager.current_steps < 0 || g_MotorManager.current_steps > CALI_MOTOR_THRESHOLD)
            g_MotorManager.total_steps -= g_MotorManager.current_steps;
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
    }
    else {
        g_MotorManager.status = STOP;
    }
}


void ringMotor() {
    MOTOR.setSpeed(500);
    for (int i = 0; i<100; i++) {
        MOTOR.step(1);
        MOTOR.step(-1);
    }
    MOTOR.setSpeed(600);
    for (int i = 0; i<100; i++) {
        MOTOR.step(1);
        MOTOR.step(-1);
    }
    MOTOR.setSpeed(650);
    for (int i = 0; i<100; i++) {
        MOTOR.step(1);
        MOTOR.step(-1);
    }
    MOTOR.setSpeed(500);
}


void triggerTask() {
    TRIGGERED = digitalRead(TRIGGER);
    if (TRIGGERED) {
        g_LEDManager.ledMotor = MOTOR_LED_FLASH;
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