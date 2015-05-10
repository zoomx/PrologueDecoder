#include <RF_Switch.h>
#include <PrologueDecoder.h>

PrologueDecoder decoder;
RF_Switch rfSwitch(2);

int pw;
bool high;
PrologueData data;

void setup(){
	pinMode(2,INPUT);
	Serial.begin(115200);
	Serial.println("Connected");
}

void loop(){
	if ((pw = rfSwitch.pulse(&high))){
		if (decoder.pulse(pw, high)){
			data = decoder.getData();
			Serial.println(data.temp);
			Serial.println(data.ID);
			Serial.println(data.rollingID);
			Serial.println(data.battery);
			Serial.println(data.button);
		}
	}

}
