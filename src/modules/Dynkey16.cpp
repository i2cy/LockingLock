/***------------------------------------***
 Project: LockingLock
 Filename: Dynkey16.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/18
***------------------------------------***/


#include "Dynkey16.h"
#include "MD5Builder.h"
#include "time.h"


MD5Builder MD5;


Dynkey16::Dynkey16(Key_t *key, uint8_t flush_times, int16_t divide) {
    this->key = key;
    this->flush_times = flush_times;
    this->divide = divide;
}


Dynkey16::Dynkey16(Key_t *key, uint8_t flush_times) {
    this->key = key;
    this->flush_times = flush_times;
    this->divide = 60.0f;
}


Dynkey16::Dynkey16(Key_t *key, int16_t divide) {
    this->key = key;
    this->flush_times = 1;
    this->divide = divide;
}


Dynkey16::Dynkey16(Key_t *key) {
    this->key = key;
    this->flush_times = 1;
    this->divide = 60.0f;
}


void Dynkey16::keygen(Key_t *dst) {
    keygen(dst, 0);
}


void Dynkey16::keygen(Key_t *dst, int8_t offset) {
    sub_key_unit = dst->value;

    time(&timestamp);
    timestamp += time_offset;

    dst->timestamp = timestamp;
    timestamp = (timestamp / divide) + offset;

    MD5.begin();
    MD5.add((uint8_t *) &timestamp, 4);
    MD5.calculate();
    MD5.getBytes(sub_key_unit);

    //printMD5("Time Unit: ", sub_key_unit);

    MD5.begin();
    MD5.add(key->value, key->len);
    MD5.calculate();
    MD5.getBytes(key_unit);

    //printMD5("Key Unit: ", key_unit);

    MD5.begin();
    MD5.add(sub_key_unit, 16);
    MD5.add(key_unit, 16);
    MD5.calculate();
    MD5.getBytes(sub_key_unit);

    //printMD5("Sub Key Unit: ", sub_key_unit);

    for (uint8_t i = 0; i < flush_times; i++) {
        MD5.begin();
        MD5.add(sub_key_unit, 16);
        MD5.calculate();
        MD5.getBytes(sub_key_unit);
        revertMD5Array(sub_key_unit);

        for (uint8_t j = 0; j < 3; j++) {
            conv_core[j] = (sub_key_unit[j] + (int8_t) ((sub_key_unit[j] % 32) / (divide % 4 + 1))) % 255 + 1;
        }

        for (uint8_t j = 3; j < 16 - 2; j++) {
            conv_temp = 0;
            for (uint8_t k = 0; k < 3; k++) {
                conv_temp += sub_key_unit[j + k] * conv_core[k];
            }
            sub_key_unit[j] = conv_temp;
        }

        MD5.begin();
        MD5.add(sub_key_unit, 16);
        MD5.add(conv_core, 3);
        MD5.add(key_unit, 16);
        MD5.calculate();
        MD5.getBytes(sub_key_unit);
    }

    memcpy(dst->value, sub_key_unit, 16);

    //printMD5("Finnal Key: ", sub_key_unit);
}


void Dynkey16::revertMD5Array(uint8_t *target) {
    uint8_t temp, ind = 15;

    for (uint8_t i = 0; i < 8; i++) {
        temp = target[i];
        target[i] = target[ind];
        target[ind] = temp;
        ind--;
    }
}


void Dynkey16::setTimeOffset(int32_t offset) {
    this->time_offset = offset;
}


void Dynkey16::charToHex(uint8_t *dst, uint8_t *source, uint16_t len) {
    char hex_table[] = {'0', '1', '2', '3',
                        '4', '5', '6', '7',
                        '8', '9', 'A', 'B',
                        'C', 'D', 'E', 'F'};
    while (len--) {
        *(dst++) = hex_table[(*source) >> 4];
        *(dst++) = hex_table[(*source++) & 0x0f];
    }
}


void Dynkey16::printMD5(const char *title, uint8_t *keychain) {
    Serial.print(title);
    charToHex(debug_array, keychain, 16);
    debug_array[32] = 0x00;
    Serial.print((char *) debug_array);
    Serial.print("\n");
}
