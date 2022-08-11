#include <Arduino.h>
#include "modules/htsocket.h"
#include "modules/config.h"
#include "modules/serial.h"
#include "modules/mqtts.h"
#include "modules/motor.h"
#include "modules/mpu6050.h"
#include "modules/kernel.h"


void setup() {
    // 初始化串口
    initSerial();
    // 初始化Wifi及MQTT
    initMqtt();
    // 初始化电机驱动
    initMotor();
    // 初始化MPU6050
    initMPU6050();
    // 初始化实时内核
    initKernel();
}

void loop() {
    // 非实时循环
    MqttTask();
    serialEvent();

    // 实时循环（1KHz）
    kernelTask();
}
