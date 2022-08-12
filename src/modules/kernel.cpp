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
bool FLAG_KERNEL2_RUN = false;


void kernelTickInterrupt() {
    Kernel_Cnt = Kernel_Cnt < 50000 ? Kernel_Cnt + 1 : 0;
    FLAG_KERNEL_RUN = true;
    FLAG_KERNEL2_RUN = true;
}


void initKernel() {
    Kernel_Tick = timerBegin(0, CPU_CLK_FREQ / 1000000ul, true);
    timerAttachInterrupt(Kernel_Tick, kernelTickInterrupt, true);
    timerAlarmWrite(Kernel_Tick, 500, true); // 设定定时器每0.5ms触发一次
    timerAlarmEnable(Kernel_Tick);

    // 等待200ms
    delay(200);
    // 将实时内核运行在第二核心上
    xTaskCreatePinnedToCore(kernelLoopCPU2,
                            "RT_KERNEL",
                            32768,
                            NULL,
                            1,
                            NULL,
                            1
    );
}

// 实时内核（2KHz）
void kernelLoopCPU2(void *pvParameters) {
    while (true) {
        if (!FLAG_KERNEL2_RUN) {
            delayMicroseconds(10);
            continue;
        }
        else FLAG_KERNEL2_RUN = false;

        // 1000Hz
        if (!(Kernel_Cnt % 2)) {
            mpu6050RtTask(0.002);
        }

        // 20Hz
        if (!(Kernel_Cnt % 100)) {
            mpu6050CaliEventTask(0.05);
        }
    }
}

// 实时内核（2KHz）
void kernelTask() {
    if (!FLAG_KERNEL_RUN) {
        delayMicroseconds(10);
        return;
    }
    else FLAG_KERNEL_RUN = false;

    // 1000Hz
    if (!(Kernel_Cnt % 2)) {
        mpu6050DebugTask();
    }
}
