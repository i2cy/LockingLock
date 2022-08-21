/***------------------------------------***
 Project: LockingLock
 Filename: Dynkey.h
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/18
***------------------------------------***/


#ifndef LOCKINGLOCK_DYNKEY_H
#define LOCKINGLOCK_DYNKEY_H

#include "Arduino.h"


typedef struct {
    uint8_t value[64];
    uint8_t len;
    uint32_t timestamp;
} Key_t;


void initDynkey();


class Dynkey {

public:
    Dynkey(Key_t *key, uint8_t flush_times, float multiplier);

    Dynkey(Key_t *key, uint8_t flush_times);

    Dynkey(Key_t *key, float multiplier);

    Dynkey(Key_t *key);

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

private:
    Key_t *key{};
    float multiplier{};
    uint8_t flush_times{};

    /*
     * revert array
     */
    static void revertMD5Array(uint8_t *target);

};


#endif //LOCKINGLOCK_DYNKEY_H
