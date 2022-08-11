#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include "modules/motor.h"


#define MOTOR_A     11
#define MOTOR_B     9
#define MOTOR_C     8
#define MOTOR_D     10



Stepper MOTOR = Stepper(64,
                        MOTOR_A, MOTOR_B,
                        MOTOR_C, MOTOR_D);

void initMotor()
{
    // initialize step motor
    pinMode(MOTOR_A, OUTPUT);
    pinMode(MOTOR_B, OUTPUT);
    pinMode(MOTOR_C, OUTPUT);
    pinMode(MOTOR_D, OUTPUT);

    testMotor();
}


void testMotor() {
    MOTOR.setSpeed(240);
    for (int i = 0; i<100; i++) {
        MOTOR.step(1);
        MOTOR.step(-1);
    }
    MOTOR.setSpeed(300);
    for (int i = 0; i<100; i++) {
        MOTOR.step(1);
        MOTOR.step(-1);
    }
    MOTOR.setSpeed(500);
    for (int i = 0; i<100; i++) {
        MOTOR.step(1);
        MOTOR.step(-1);
    }
    MOTOR.setSpeed(240);
}


void MotorTask()
{
    //MOTOR.setSpeed(GyY);
    MOTOR.step(200);
    MOTOR.step(-200);
}