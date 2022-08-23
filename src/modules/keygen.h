/***------------------------------------***
 Project: LockingLock
 Filename: keygen.h
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/14
***------------------------------------***/


#ifndef LOCKINGLOCK_KEYGEN_H
#define LOCKINGLOCK_KEYGEN_H

#endif //LOCKINGLOCK_KEYGEN_H


#include <Arduino.h>


typedef struct {
    uint8_t tri_keychain[3][8];
} KeyManager_t;


void initKeygen();

void dynkeyDebugTask();

void setDynkey16TimeOffset(int32_t offset);
