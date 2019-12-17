/*
  Prologue
  by c-math
  RF protocol decoder for the Prologue temperature sensor
  https://github.com/c-math/PrologueDecoder

  Added humidity decoding by zoomx
  2017/12/02
*/



#include "RF_Switch.h"   //https://github.com/c-math/RF_Switch
#include "PrologueDecoder.h"

PrologueDecoder decoder;
RF_Switch rfSwitch(2);

int pw;
bool high;
PrologueData data;

void setup() {
  pinMode(2, INPUT);
  Serial.begin(115200);
  Serial.println("Prologue");
}

void loop() {
  if ((pw = rfSwitch.pulse(&high))) {
    if (decoder.pulse(pw, high)) {
      data = decoder.getData();
      Serial.println(data.temp);
      Serial.println(data.humidity);
      Serial.println(data.ID);
      Serial.println(data.rollingID);
      Serial.println(data.battery);
      Serial.println(data.button);
    }
  }

}
