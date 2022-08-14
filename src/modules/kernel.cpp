/***------------------------------------***
 Project: LockingLock
 Filename: kernel.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/


#include <Arduino.h>
#include "kernel.h"
#include "mpu6050.h"
#include "led.h"
#include "mqtts.h"
#include "esp_task_wdt.h"
#include "motor.h"


hw_timer_t *Kernel_Tick = NULL;
uint16_t Kernel_Cnt = 0;
uint16_t Kernel2_Cnt = 0;

bool FLAG_KERNEL_RUN = false;
bool FLAG_KERNEL2_RUN = false;


void kernelTickInterrupt() {
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
                            65536,
                            NULL,
                            1,
                            NULL,
                            0
    );
}

// 实时内核（2KHz）(CPU-0)
void kernelLoopCPU2(void *pvParameters) {
    while (true) {
        if (!FLAG_KERNEL2_RUN) {
            delayMicroseconds(1);
            continue;
        }
        else FLAG_KERNEL2_RUN = false;

        // 500Hz
        if (!(Kernel2_Cnt % 4)) {
            mpu6050DebugTask();
            mpu6050VibeProcessTask(0.002);
            //mpu6050RtTask(0.002);
        }

        // 333Hz
        if (!(Kernel2_Cnt % 6)) {
            motorTask();
            triggerTask();
        }

        // 200Hz
        if (!(Kernel_Cnt % 10)) {
            motorSequenceTask();
        }

        // 50Hz
        if (!(Kernel2_Cnt % 40)) {
            ledEventTask();
            //mpu6050CaliEventTask(0.02);
        }

        // 1Hz
        if (!(Kernel2_Cnt % 2000)) {
            vTaskDelay(1);
            Kernel2_Cnt = Kernel2_Cnt + 2;
            if (Kernel2_Cnt > 60000) Kernel2_Cnt -= 60000;
        }

        Kernel2_Cnt = Kernel2_Cnt < 60000 ? Kernel2_Cnt + 1 : 0;
    }
}

// 实时内核（2KHz）(CPU-1)
void kernelTask() {
    if (!FLAG_KERNEL_RUN) {
        delayMicroseconds(1);
        return;
    }
    else FLAG_KERNEL_RUN = false;

    // 666Hz
    if (!(Kernel_Cnt % 3)) {
        mpu6050RtTask(0.002);
    }

    // 20Hz
    if (!(Kernel_Cnt % 100)) {
        mpu6050CaliEventTask(0.05);
        //ledEventTask();
        //motorTask();
    }

    // 10Hz
    if (!(Kernel_Cnt % 200)) {
        reconnectWifiEventTask();
        clientReconnectEventTask();
    }

    Kernel_Cnt = Kernel_Cnt < 60000 ? Kernel_Cnt + 1 : 0;
}
