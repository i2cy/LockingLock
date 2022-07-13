//
// Created by i2cy on 2022/7/11.
//

#define PACKAGE_HEADER  0xaa
#define RESERVED_VALUE  0xff
#define MAX_TRAIL       32

#include "htsocket.h"

// HTSocket 包解析
uint8_t readHtpack(uint8_t *ret, uint8_t addr, uint8_t fcode) {
    uint8_t raw;
    uint8_t checksum;
    uint8_t length = 0;

    for (int i2 = 0; i2 < MAX_TRAIL; i2++) {
        checksum = 0;
        // 对齐帧头
        raw = Serial.read();
        if (raw != PACKAGE_HEADER) continue;
        checksum += raw;

        // 筛选地址
        raw = Serial.read();
        if (raw != addr and addr != RESERVED_VALUE) continue;
        checksum += raw;

        // 筛选功能字
        raw = Serial.read();
        if (raw != fcode and fcode != RESERVED_VALUE) continue;
        checksum += raw;

        // 获取帧长度
        raw = Serial.read();
        //Serial.println(raw);
        checksum += raw;

        // 获取数据部分
        for (int i = 0; i < raw; i++) {
            ret[i] = Serial.read();
            //Serial.print((char)ret[i]);
            checksum += ret[i];
        }
        length = raw;

        // 校验
        raw = Serial.read();
        if (checksum != raw) {
            length = 0;
            continue;
        }

        break;
    }

    return length;
}

// HTSocket 包发送
void sendHtpack(uint8_t *data, uint8_t addr, uint8_t fcode, uint8_t length) {
    uint8_t checksum = 0;

    // 发送帧头
    Serial.print((char)PACKAGE_HEADER);
    checksum += PACKAGE_HEADER;

    // 发送地址
    Serial.print((char)addr);
    checksum += addr;

    // 发送功能字
    Serial.print((char)fcode);
    checksum += fcode;

    // 发送帧长度
    Serial.print((char)length);
    checksum += length;

    // 发送数据部分
    for (int i = 0; i < length; i++) {
        Serial.print((char)data[i]);
        checksum += data[i];
    }

    // 发送校验和
    Serial.print((char)checksum);
}
