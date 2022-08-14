//
// Created by i2cy on 2022/7/16.
//

#ifndef LOCKINGLOCK_MOTOR_H
#define LOCKINGLOCK_MOTOR_H

#endif //LOCKINGLOCK_MOTOR_H


typedef enum {
    STOP = 0,
    IDLE,
    RETRACT,
    RELEASE,
} em_MotorStatus_t;


typedef struct {
    uint32_t total_steps;
    int32_t current_steps;
    em_MotorStatus_t status;
} MotorManager_t;


void ringMotor();
void initMotor();
void setMotorStop();
void motorTask();
void triggerTask();
