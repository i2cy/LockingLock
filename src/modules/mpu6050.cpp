/***------------------------------------***
 Project: LockingLock
 Filename: mpu6050.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/

#include "Wire.h"
#include "mpu6050.h"
#include <Arduino.h>


#define SDA         12
#define SCL         13


const int MPU_addr = 0x68;      // I2C address of the MPU-6050

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;


void initMPU6050() {
    // initialize MPU6050
    Wire.begin(SDA, SCL);
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    //Serial.begin(115200);
}

void mpu6050DebugTask() {
    Serial.print(AcX);
    Serial.print(","); Serial.print(AcY);
    Serial.print(","); Serial.print(AcZ);
    Serial.print(","); Serial.print(Tmp / 340.00 + 36.53); //equation for temperature in degrees C from datasheet
    Serial.print(","); Serial.print(GyX);
    Serial.print(","); Serial.print(GyY);
    Serial.print(","); Serial.println(GyZ);
}

// 500Hz运行
void mpu6050RtTask(float dt) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, 1); // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

    mpu6050DebugTask();
}