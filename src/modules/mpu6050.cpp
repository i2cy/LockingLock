/***------------------------------------***
 Project: LockingLock
 Filename: mpu6050.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/

#include "Wire.h"
#include "mpu6050.h"
#include "htsocket.h"
#include "led.h"
#include "motor.h"


#define SDA                     12
#define SCL                     13

#define PWR_MGMT                0x6B
#define SMPLRT_DIV              0x19    // 陀螺仪采样率，典型值：0x07(125Hz)
#define CONFIGL                 0x1A    // 低通滤波频率，典型值：0x06(5Hz)
#define GYRO_CONFIG             0x1B    // 陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define ACCEL_CONFIG            0x1C    // 加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define ACCEL_XOUT_H            0x3B    // 数据寄存器偏移量
#define PRODUCT_ID_ADDR         0x75    // 设备ID地址
#define PRODUCT_ID              0x68

#define HT_ADDR                 0xcb
#define HT_FUNC                 0x01

#define CALI_WINDOW_LPF         0.005f  // 传感器零点指标LPF系数
#define CALI_ACC_THRESHOLD      20.0f   // 加速度计校准事件触发阈值
#define CALI_GYRO_THRESHOLD     20.0f   // 陀螺仪校准事件触发阈值

#define KNOCK_TRIG_THRESHOLD    20      // 敲击检测阈值
#define KNOCK_AMP_THRESHOLD     30      // 敲击检测幅值阈值
#define KNOCK_YMAX_GAMMA        0.98f   // 敲击最大幅值衰减系数
#define KNOCK_LPF               0.4f    // 振动幅值低通滤波系数
#define KNOCK_LPF_LEVEL         5       // 振动幅值低通滤波器阶数
#define KNOCK_TIMEOUT           1500    // 敲击超时计数
#define KNOCK_DEATHROOM         36      // 敲击死区
#define KNOCK_SPLIT_THRESHOLD   325     // 敲击密码组间隔时间


extern LEDManager_t g_LEDManager;
extern MotorManager_t g_MotorManager;

const int MPU_addr = 0x68;          // I2C address of the MPU-6050

bool FLAG_MPU6050_READY = false;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float OFFSETS[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float DEBUG = 0.0;
uint8_t MPU6050_WATCHDOG_COUNTDOWN = 200;

VibeManager_t g_VibeManager;


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

    //Serial.print("ASD");
}


void feedMPU6050Wd() {
    MPU6050_WATCHDOG_COUNTDOWN = 200;
}

// 20Hz
void watchdogMPU6050Task() {
    bool condition;
    if (MPU6050_WATCHDOG_COUNTDOWN) {
        MPU6050_WATCHDOG_COUNTDOWN--;
    }
    else {
        //esp_restart();
        initMPU6050();
        feedMPU6050Wd();
    }

    condition = AcX == 0.0 && AcY == 0.0 && AcZ == 0.0;
    if (condition) {
        FLAG_MPU6050_READY = false;
        g_LEDManager.ledKnock = KNOCK_LED_FLASH;
    }
    else {
        FLAG_MPU6050_READY = true;
        feedMPU6050Wd();
    }
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
    g_VibeManager.expire_countdown = 0;
    g_VibeManager.current_sequence = 0;
    g_VibeManager.index = 0;
    g_VibeManager.triggered = false;
    for (float & i : g_VibeManager.lpf) i = 0.0;
    for (float & i : g_VibeManager.amplitudes) i = 0.0;

    Wire.begin(SDA, SCL);
    delay(200);

    writeMpu6050Byte(PWR_MGMT, 0x80);       // 复位
    delay(200);

    writeMpu6050Byte(SMPLRT_DIV, 0x00);     // 设置陀螺仪采样率(1000Hz)
    delay(10);
    writeMpu6050Byte(PWR_MGMT, 0x00);       // 设置设备时钟源
    delay(10);
    writeMpu6050Byte(CONFIGL, 0x03);        // 设置LPF（截止频率44Hz）
    delay(10);
    writeMpu6050Byte(GYRO_CONFIG, 0x18);    // 设置陀螺仪量程（2000°/s，不自检）
    delay(10);
    writeMpu6050Byte(ACCEL_CONFIG, 0x10);   // 设置加速度计量程（8G，不自检）
    delay(10);

    recalibrateMPU6050();

    FLAG_MPU6050_READY = true;
}


void mpu6050DebugTask() {
    if (!FLAG_MPU6050_READY) return;

    uint16_t buf[7];
    buf[0] = g_VibeManager.current_sequence;
    buf[1] = g_VibeManager.expire_countdown;
    buf[2] = AcZ;
    buf[3] = (int16_t) (Tmp / 340.00 + 36.53);
    buf[4] = g_VibeManager.sequence[0];
    buf[5] = (int16_t)DEBUG;
    buf[6] = (int16_t)g_VibeManager.amplitudes[g_VibeManager.index];

    sendHtpack((uint8_t *) &buf, HT_ADDR, HT_FUNC, sizeof(buf));
}

// 20Hz 传感器校准事件监测
void mpu6050CaliEventTask(float dt) {
    if (!FLAG_MPU6050_READY) return;

    OFFSETS[0] += CALI_WINDOW_LPF * (float)AcX;
    OFFSETS[1] += CALI_WINDOW_LPF * (float)AcY;
    OFFSETS[2] += CALI_WINDOW_LPF * (float)AcZ;
    OFFSETS[3] += CALI_WINDOW_LPF * (float)GyX;
    OFFSETS[4] += CALI_WINDOW_LPF * (float)GyY;
    OFFSETS[5] += CALI_WINDOW_LPF * (float)GyZ;
}

// 500Hz mpu6050采样任务
void mpu6050RtTask(float dt) {
    static float ymax = 0.0;
    uint8_t cnt = 0;
    readMpu6050Data();

    if (!FLAG_MPU6050_READY) return;

    for (float & i : g_VibeManager.lpf) {
        if (cnt) {
            i += KNOCK_LPF * (g_VibeManager.lpf[cnt - 1] - i);
        }
        else {
            i += KNOCK_LPF * ((float)abs(AcZ) - i);
        }
        cnt++;
    }

    ymax = g_VibeManager.lpf[KNOCK_LPF_LEVEL] > ymax ? g_VibeManager.lpf[KNOCK_LPF_LEVEL] : ymax * KNOCK_YMAX_GAMMA;

    g_VibeManager.amplitudes[g_VibeManager.index] = ymax;
    g_VibeManager.index = g_VibeManager.index < 21 ? g_VibeManager.index + 1 : 0;
}

// 500Hz mpu6050震动感知任务
void mpu6050VibeProcessTask(float dt) {
    static bool step = true;

    int8_t imin, imax = 0;
    bool condition_a, condition_b = false;
    if (!FLAG_MPU6050_READY) return;

    if (!g_VibeManager.expire_countdown) {
        if (g_VibeManager.current_sequence == 7) {
            // 密钥验证
        }
        g_VibeManager.current_sequence = 0;
        g_VibeManager.sequence[0] = 0;
    }
    else {
        g_VibeManager.expire_countdown--;
    }

    for (int8_t i; i < 22; i++) {
        if (g_VibeManager.amplitudes[i] > g_VibeManager.amplitudes[imax])
            imax = i;
        if (g_VibeManager.amplitudes[i] < g_VibeManager.amplitudes[imin])
            imin = i;
    }

    condition_a = (g_VibeManager.amplitudes[imax] - g_VibeManager.amplitudes[imin]) > KNOCK_TRIG_THRESHOLD;
    condition_b = g_VibeManager.amplitudes[imax] > KNOCK_AMP_THRESHOLD;

    DEBUG = g_VibeManager.amplitudes[imax] - g_VibeManager.amplitudes[imin];

    if (imax >= g_VibeManager.index) imax -= 22;
    if (imin >= g_VibeManager.index) imin -= 22;

    if ( condition_b && imax > imin) {
        if (!g_VibeManager.triggered) {
            g_VibeManager.sequence[g_VibeManager.current_sequence]++;
            step = false;
        }
        g_VibeManager.expire_countdown = KNOCK_TIMEOUT;
        g_VibeManager.triggered = true;
        g_LEDManager.ledKnock = KNOCK_LED_ON;
    }
    else {
        if ((KNOCK_TIMEOUT - g_VibeManager.expire_countdown) > KNOCK_DEATHROOM) {
            g_VibeManager.triggered = false;
            g_LEDManager.ledKnock = KNOCK_LED_OFF;
        }
        if (!step && (KNOCK_TIMEOUT - g_VibeManager.expire_countdown) > KNOCK_SPLIT_THRESHOLD) {
            g_VibeManager.current_sequence =
                    g_VibeManager.current_sequence < 7 ? g_VibeManager.current_sequence + 1 : 0;
            g_VibeManager.sequence[g_VibeManager.current_sequence] = 0;
            step = true;
        }
    }
}
