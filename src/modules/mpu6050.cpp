/***------------------------------------***
 Project: LockingLock
 Filename: mpu6050.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/

#include "Wire.h"
#include "mpu6050.h"
#include "htsocket.h"


#define SDA                 12
#define SCL                 13

#define PWR_MGMT            0x6B
#define SMPLRT_DIV          0x19    // 陀螺仪采样率，典型值：0x07(125Hz)
#define CONFIGL             0x1A    // 低通滤波频率，典型值：0x06(5Hz)
#define GYRO_CONFIG         0x1B    // 陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define ACCEL_CONFIG        0x1C    // 加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define ACCEL_XOUT_H        0x3B    // 数据寄存器偏移量

#define HT_ADDR             0xcb
#define HT_FUNC             0x01

#define CALI_WINDOW_LPF     0.02f  // 传感器零点指标LPF系数
#define CALI_ACC_THRESHOLD  20.0f   // 加速度计校准事件触发阈值
#define CALI_GYRO_THRESHOLD 20.0f   // 陀螺仪校准事件触发阈值


const int MPU_addr = 0x68;      // I2C address of the MPU-6050

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float OFFSETS[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};


void writeMpu6050Byte(uint8_t register_addr, uint8_t data) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(register_addr);
    Wire.write(data);
    Wire.endTransmission(true);
}


void readMpu6050Data() {
    Wire.beginTransmission(MPU_addr);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, 1); // request a total of 14 registers
    AcX = (Wire.read() << 8 | Wire.read()) - OFFSETS[0]; // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = (Wire.read() << 8 | Wire.read()) - OFFSETS[1]; // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = (Wire.read() << 8 | Wire.read()) - OFFSETS[2]; // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = (Wire.read() << 8 | Wire.read()) - OFFSETS[3]; // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = (Wire.read() << 8 | Wire.read()) - OFFSETS[4]; // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = (Wire.read() << 8 | Wire.read()) - OFFSETS[5]; // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}


void readMpu6050RawData() {
    Wire.beginTransmission(MPU_addr);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, 1); // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}


void recalibrateMPU6050() {
    for (float & i : OFFSETS) i = 0.0;

    // 计算偏移量
    for (uint16_t i = 0; i < 1000; i++) {
        delayMicroseconds(500);
        readMpu6050RawData();
        OFFSETS[0] += 0.001f * (float) AcX;
        OFFSETS[1] += 0.001f * (float) AcY;
        OFFSETS[2] += 0.001f * (float) AcZ;
        OFFSETS[3] += 0.001f * (float) GyX;
        OFFSETS[4] += 0.001f * (float) GyY;
        OFFSETS[5] += 0.001f * (float) GyZ;
    }
}


void initMPU6050() {
    Wire.begin(SDA, SCL);

    writeMpu6050Byte(PWR_MGMT, 0x80);       // 复位
    writeMpu6050Byte(SMPLRT_DIV, 0x00);     // 设置陀螺仪采样率(1000Hz)
    writeMpu6050Byte(PWR_MGMT, 0x00);       // 设置设备时钟源
    writeMpu6050Byte(CONFIGL, 0x00);        // 设置LPF（截止频率44Hz）
    writeMpu6050Byte(GYRO_CONFIG, 0x18);    // 设置陀螺仪量程（2000°/s，不自检）
    writeMpu6050Byte(ACCEL_CONFIG, 0x10);   // 设置加速度计量程（8G，不自检）

    // 抛弃前2000个数据
    for (uint16_t i = 0; i < 2000; i++) {
        delay(1);
        readMpu6050RawData();
    }
    // 计算偏移量
    for (uint16_t i = 0; i < 1000; i++) {
        delay(1);
        readMpu6050RawData();
        OFFSETS[0] += 0.001f * (float) AcX;
        OFFSETS[1] += 0.001f * (float) AcY;
        OFFSETS[2] += 0.001f * (float) AcZ;
        OFFSETS[3] += 0.001f * (float) GyX;
        OFFSETS[4] += 0.001f * (float) GyY;
        OFFSETS[5] += 0.001f * (float) GyZ;
    }
}


void mpu6050DebugTask() {
    uint16_t buf[7];
    buf[0] = AcX;
    buf[1] = AcY;
    buf[2] = AcZ;
    buf[3] = (int16_t) (Tmp / 340.00 + 36.53);
    buf[4] = GyX;
    buf[5] = GyY;
    buf[6] = GyZ;

    sendHtpack((uint8_t *) &buf, HT_ADDR, HT_FUNC, sizeof(buf));
}

// 20Hz 传感器校准事件监测
void mpu6050CaliEventTask(float dt) {
    static float window[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    for (uint8_t i = 0; i < 3; i++) {
        if (window[i] > CALI_ACC_THRESHOLD) {
            recalibrateMPU6050();
            for (float & i2 : window) i2 = 0.0;
            return;
        }
    }

    for (uint8_t i = 3; i < 6; i++) {
        if (window[i] > CALI_GYRO_THRESHOLD) {
            recalibrateMPU6050();
            for (float & i2 : window) i2 = 0.0;
            return;
        }
    }

    window[0] += CALI_WINDOW_LPF * ((float)AcX - window[0]);
    window[1] += CALI_WINDOW_LPF * ((float)AcY - window[1]);
    window[2] += CALI_WINDOW_LPF * ((float)AcZ - window[2]);
    window[3] += CALI_WINDOW_LPF * ((float)GyX - window[3]);
    window[4] += CALI_WINDOW_LPF * ((float)GyY - window[4]);
    window[5] += CALI_WINDOW_LPF * ((float)GyZ - window[5]);
}

// 500Hz mpu6050采样任务
void mpu6050RtTask(float dt) {
    readMpu6050Data();

    //mpu6050DebugTask();
}