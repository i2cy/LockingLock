/***------------------------------------***
 Project: LockingLock
 Filename: kernel.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/


#include <Arduino.h>
#include "kernel.h"
#include "mpu6050.h"


hw_timer_t *Kernel_Tick = NULL;
uint16_t Kernel_Cnt = 0;

bool FLAG_KERNEL_RUN = false;


void kernelTickInterrupt() {
    Kernel_Cnt = Kernel_Cnt < 50000 ? Kernel_Cnt + 1 : 0;
    FLAG_KERNEL_RUN = true;
}


void initKernel() {
    Kernel_Tick = timerBegin(0, 240, true);
    timerAttachInterrupt(Kernel_Tick, kernelTickInterrupt, true);
    timerAlarmWrite(Kernel_Tick, 1000, true); // 设定定时器每1ms触发一次
    timerAlarmEnable(Kernel_Tick);
}

// 实时内核（1KHz）
void kernelTask() {
    if (!FLAG_KERNEL_RUN) return;
    else FLAG_KERNEL_RUN = false;

    // 500Hz
    if (!(Kernel_Cnt % 2)) {
        mpu6050RtTask(0.002);
    }
}
