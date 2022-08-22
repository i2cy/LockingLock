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


typedef enum {
    LOCKED = 0,
    LOCKING,
    WAITING_BY_CMD,
    UNLOCK_BY_CMD,
    HOLD_BY_CMD,
    CALI_BY_CMD,
    HOLD_CALI_BY_CMD,
    CALI_FINISH,
    UNLOCK_BY_PSK,
    HOLD_BY_PSK
} em_MotorSequence_t;


typedef struct {
    uint32_t total_steps;
    int32_t current_steps;
    uint32_t countdown;
    em_MotorStatus_t status;
    em_MotorSequence_t sequence;
    bool cali_enabled;
} MotorManager_t;


void ringMotor();
void initMotor();

void setMotorStop();
void setMotorCaliOffset();
void setMotorCmdUnlock();

void motorTask();
void triggerTask();
void motorSequenceTask();
