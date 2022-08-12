/***------------------------------------***
 Project: LockingLock
 Filename: mpu6050.h
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/


#ifndef LOCKINGLOCK_MPU6050_H
#define LOCKINGLOCK_MPU6050_H

#endif //LOCKINGLOCK_MPU6050_H

// 初始化MPU6050
void initMPU6050();
// 实时任务（500Hz）
void mpu6050RtTask(float dt);
// 实时任务（20Hz）
void mpu6050CaliEventTask(float dt);
void mpu6050DebugTask();
