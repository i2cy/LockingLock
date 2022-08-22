/***------------------------------------***
 Project: LockingLock
 Filename: mpu6050.h
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/


#ifndef LOCKINGLOCK_MPU6050_H
#define LOCKINGLOCK_MPU6050_H

#endif //LOCKINGLOCK_MPU6050_H


typedef struct {
    uint16_t expire_countdown;  // 敲击超时倒计时
    uint8_t current_sequence;   // 敲击序列索引
    uint8_t sequence[4];        // 敲击序列

    uint8_t index;              // 波形队列头索引
    bool triggered;             // 当前敲击状态
    float lpf[6];               // 六阶低通滤波器
    float amplitudes[22];       // 波形幅值队列
} VibeManager_t;


// 初始化MPU6050
void initMPU6050();
// 实时任务（500Hz）
void mpu6050RtTask(float dt);
// 实时任务（20Hz）
void mpu6050CaliEventTask(float dt);
// 实时任务（500Hz）
void mpu6050VibeProcessTask(float dt);
// 实时任务（20Hz）
void watchdogMPU6050Task();
// 实时任务（500Hz）
void mpu6050DebugTask();
