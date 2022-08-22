//
// Created by Adminadorstr on 2022/8/11.
//

#ifndef LOCKINGLOCK_MQTTS_H
#define LOCKINGLOCK_MQTTS_H

#endif //LOCKINGLOCK_MQTTS_H

#include "Arduino.h"


typedef struct {
    uint8_t flow_cnt;
    uint8_t cmd_id;
    uint16_t length;
} MqttCmdHeader_t;


bool setupMQTT();

void reconnectWifiEventTask();

void initMqtt();

void MqttTask();

void clientReconnectEventTask();

void sendCaliOffset(uint32_t offset);
