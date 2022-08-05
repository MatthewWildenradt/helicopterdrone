/* I2CSlave.cpp */

#include "I2CSlave.h"

void receiveEvent(int howMany){

  while(Wire.available()){
    doseur[receiveByte] = Wire.read();
    receiveByte++;
  }
  
  Serial.println(doseur[0], HEX);
  Serial.println(doseur[1], HEX);

  receiveByte=0;
  
}

void requestEvent(void){
  Wire.write("done");
}

void wireOnReceive(void){
  Wire.onReceive(receiveEvent);
}

void wireOnRequest(void){
  Wire.onRequest(requestEvent);
}