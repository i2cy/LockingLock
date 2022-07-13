//
// Created by i2cy on 2022/7/11.
//

#ifndef ESP32S3_MQTTS_TEST_HTSOCKET_H
#define ESP32S3_MQTTS_TEST_HTSOCKET_H

#endif //ESP32S3_MQTTS_TEST_HTSOCKET_H

#include <Arduino.h>


uint8_t readHtpack(uint8_t *ret, uint8_t addr, uint8_t fcode);
void sendHtpack(uint8_t *data, uint8_t addr, uint8_t fcode, uint8_t length);
