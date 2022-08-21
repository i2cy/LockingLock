#include <Arduino.h>
#include "modules/htsocket.h"
#include "modules/config.h"
#include "modules/serial.h"
#include "modules/mqtts.h"
#include "modules/motor.h"
#include "modules/mpu6050.h"
#include "modules/kernel.h"
#include "modules/led.h"
#include "modules/keygen.h"


void setup() {
    // 设置CPU时钟频率
    setCpuFrequencyMhz(240);
    // 初始化串口
    initSerial();
    // 初始化LED
    initLED();
    // 初始化动态密钥生成器
    initKeygen();
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

    // CPU1实时循环
    kernelTask();
}
