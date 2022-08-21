/***------------------------------------***
 Project: LockingLock
 Filename: Dynkey.cpp
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/18
***------------------------------------***/


#include "Dynkey.h"
#include "MD5Builder.h"
#include "time.h"


MD5Builder MD5;


Dynkey::Dynkey(Key_t *key, uint8_t flush_times, float multiplier) {
    this->key = key;
    this->flush_times = flush_times;
    this->multiplier = multiplier;
}


Dynkey::Dynkey(Key_t *key, uint8_t flush_times) {
    this->key = key;
    this->flush_times = flush_times;
    this->multiplier = 0.01f;
}


Dynkey::Dynkey(Key_t *key, float multiplier) {
    this->key = key;
    this->flush_times = 1;
    this->multiplier = multiplier;
}


Dynkey::Dynkey(Key_t *key) {
    this->key = key;
    this->flush_times = 1;
    this->multiplier = 0.01f;
}


void Dynkey::keygen(Key_t *dst) {
    keygen(dst, 0);
}


void Dynkey::keygen(Key_t *dst, int8_t offset) {
    time_t time_val;
    uint16_t conv_temp;
    uint8_t len_temp;

    uint8_t time_unit[16], key_unit[16], sub_key_unit[16];
    uint8_t sub_key_md5[5 * 16];
    uint8_t conv_core[5], conv_res[12];

    time(&time_val);
    dst->timestamp = time_val;
    time_val = (float)time_val * multiplier + (float)offset;

    for (uint8_t i = 0; i < 16 && time_val; i++) {
        time_unit[i] = '0' + time_val % 10;
        time_val /= 10;
        len_temp++;
    }

    MD5.begin();
    MD5.add(time_unit, len_temp);
    MD5.calculate();
    MD5.getBytes(time_unit);

    MD5.begin();
    MD5.add(key->value, key->len);
    MD5.calculate();
    MD5.getBytes(key_unit);

    // first flush
    MD5.begin();
    MD5.add(time_unit, 16);
    MD5.add(key_unit, 16);
    MD5.calculate();
    MD5.getBytes(sub_key_unit);
    revertMD5Array(sub_key_unit);

    // get conv core
    for (uint8_t i; i < 3; i++) {
        conv_core[i] = sub_key_unit[i] + 1;
    }

    for (uint8_t i = 3; i < 14; i++) {
        conv_temp = 0;
        for (uint8_t j = 0; j < 3; j++) {
            conv_temp += sub_key_unit[i + j] * conv_core[j];
        }
        conv_res[i - 3] = conv_temp % 256;
    }

    MD5.begin();
    MD5.add(sub_key_unit, 3);
    MD5.add(conv_core, 3);
    MD5.calculate();
    MD5.getBytes(sub_key_unit);
    revertMD5Array(sub_key_unit);

    MD5.begin();
    MD5.add(sub_key_unit, 16);
    MD5.add(conv_res, 11);
    MD5.calculate();
    MD5.getBytes(sub_key_md5 + 16);

    MD5.begin();
    MD5.add(conv_res, 11);
    MD5.calculate();
    MD5.getBytes(sub_key_md5 + 32);

    MD5.begin();
    MD5.add(conv_res, 11);
    MD5.add(key->value, key->len);
    MD5.calculate();
    MD5.getBytes(sub_key_md5 + 48);

    MD5.begin();
    MD5.add(sub_key_unit, 16);
    MD5.add(sub_key_md5 + 16, 16);
    MD5.add(sub_key_md5 + 32, 16);
    MD5.add(sub_key_md5 + 48, 16);
    MD5.add(key_unit, 16);
    MD5.calculate();
    MD5.getBytes(sub_key_unit);

    for (uint8_t k = 1; k < flush_times; k++) {
        MD5.begin();
        MD5.add(sub_key_unit, 16);
        MD5.calculate();
        MD5.getBytes(sub_key_unit);
        revertMD5Array(sub_key_unit);

        // get conv core
        for (uint8_t i; i < 3; i++) {
            conv_core[i] = sub_key_unit[i] + 1;
        }

        for (uint8_t i = 3; i < 14; i++) {
            conv_temp = 0;
            for (uint8_t j = 0; j < 3; j++) {
                conv_temp += sub_key_unit[i + j] * conv_core[j];
            }
            conv_res[i - 3] = conv_temp % 256;
        }

        MD5.begin();
        MD5.add(sub_key_unit, 3);
        MD5.add(conv_core, 3);
        MD5.calculate();
        MD5.getBytes(sub_key_unit);
        revertMD5Array(sub_key_unit);
        memcpy(sub_key_md5, sub_key_unit, 16);

        MD5.begin();
        MD5.add(sub_key_unit, 16);
        MD5.add(conv_res, 11);
        MD5.calculate();
        MD5.getBytes(sub_key_md5 + (1 * 16));

        MD5.begin();
        MD5.add(conv_res, 11);
        MD5.calculate();
        MD5.getBytes(sub_key_md5 + (2 * 16));

        MD5.begin();
        MD5.add(conv_res, 11);
        MD5.add(key->value, key->len);
        MD5.calculate();
        MD5.getBytes(sub_key_md5 + (3 * 16));

        MD5.begin();
        MD5.add(sub_key_unit, 16);
        MD5.add(sub_key_md5 + 16, 16);
        MD5.add(sub_key_md5 + 32, 16);
        MD5.add(sub_key_md5 + 48, 16);
        MD5.add(key_unit, 16);
        MD5.calculate();
        MD5.getBytes(sub_key_unit);
        memcpy(sub_key_md5 + (4 * 16), key_unit, 16);
    }

    len_temp = 80;

    for (uint8_t k = 0; k < 4; k++) {
        for (uint8_t i = 0; i < 4; i++) {
            if (i < 2) {
                conv_core[i] = time_unit[4 * k + i];
            }
            else if (i == 2) {
                conv_core[i] = (key_unit[k] + key_unit[k + 4] + key_unit[k + 8] + key_unit[k + 12]) / 4;
                conv_core[i + 1] = time_unit[4 * k + i];
            }
            else {
                conv_core[i + 1] = time_unit[4 * k + i];
            }
        }

        for (uint8_t i = 0; i < len_temp - 4; i++) {
            conv_temp = 0;
            for (uint8_t j = 0; j < 5; j++) {
                conv_temp += sub_key_md5[i + j] * conv_core[j];
            }
            sub_key_md5[i] = conv_temp % 256;
            len_temp -= 4;
        }
    }

    memcpy(dst->value, sub_key_md5, 64);
}


void Dynkey::revertMD5Array(uint8_t *target) {
    uint8_t temp, ind = 15;

    for (uint8_t i = 0; i < 8; i++) {
        temp = target[i];
        target[i] = target[ind];
        target[ind] = temp;
        ind--;
    }
}
