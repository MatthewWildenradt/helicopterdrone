#ifndef I2CSlave_H
#define I2CSlave_H

/* I2CSlave.h */

#include <Arduino.h>
#include <Wire.h>

void receiveEvent(int howMany);
void requestEvent(void);
void wireOnReceive(void);
void wireOnRequest(void);

int receiveByte = 0;
byte myByte[2];

#endif