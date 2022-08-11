/***------------------------------------***
 Project: LockingLock
 Filename: serial.h
 Author: I2cy(i2cy@outlook.com)
 Created on: 2022/8/11
***------------------------------------***/


#ifndef LOCKINGLOCK_SERIAL_H
#define LOCKINGLOCK_SERIAL_H

#endif //LOCKINGLOCK_SERIAL_H


void initSerial();
// 串口指令处理函数
void processSerialCmd(uint8_t *cmd_t);

// 串口事件循环任务
void serialEvent();
