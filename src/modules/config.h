//
// Created by i2cy on 2022/7/13.
//

#ifndef LOCKINGLOCK_EEPROM_H
#define LOCKINGLOCK_EEPROM_H

#endif //LOCKINGLOCK_EEPROM_H

#include <Arduino.h>
#include "Preferences.h"

void readSSIDConfig(char *dst);
void writeSSIDConfig(char *ssid);
void readPSKConfig(char *dst);
void writePSKConfig(char *psk);
