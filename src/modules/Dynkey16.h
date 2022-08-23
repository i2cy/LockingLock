/***------------------------------------***
 Project: LockingLock
 Filename: Dynkey16.h
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/18
***------------------------------------***/


#ifndef LOCKINGLOCK_DYNKEY16_H
#define LOCKINGLOCK_DYNKEY16_H

#include "Arduino.h"


typedef struct {
    uint32_t timestamp;
    uint8_t value[16];
    uint8_t len;
} Key_t;


void initDynkey();


class Dynkey16 {

public:
    Dynkey16(Key_t *key, uint8_t flush_times, int16_t divide);

    Dynkey16(Key_t *key, uint8_t flush_times);

    Dynkey16(Key_t *key, int16_t divide);

    Dynkey16(Key_t *key);

    /*
     * generate key from localtime
     */
    void keygen(Key_t *dst);

    void keygen(Key_t *dst, int8_t offset);


    /*
     * match the given key
     *
     * return: bool
     */
    bool keymatch(Key_t *key);


    /*
     * set timestamp offset
     */
    void setTimeOffset(int32_t offset);


private:
    Key_t *key{};
    int16_t divide{};
    uint8_t flush_times{};
    int32_t time_offset = 0;

    time_t timestamp;
    uint8_t conv_temp;
    uint8_t key_unit[16];
    uint8_t conv_core[3];
    uint8_t *sub_key_unit;

    uint8_t debug_array[33];

    /*
     * revert array
     */
    static void revertMD5Array(uint8_t *target);

    /*
     * convert char sets to hex sets
     */
    static void charToHex(uint8_t *dst, uint8_t *source, uint16_t len);

    /*
     * print MD5 bytes
     */
    void printMD5(const char *title, uint8_t *keychain);

};


#endif //LOCKINGLOCK_DYNKEY16_H
