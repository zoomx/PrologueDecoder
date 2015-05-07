#include <RF_Switch.h>
#include <PrologueDecoder.h>

PrologueDecoder decoder;
RF_Switch rfSwitch(2);

int pw;
bool high;
Data data;

void setup(){
	pinMode(2,INPUT);
	Serial.begin(115200);
	Serial.println("Connected");
}

void loop(){

	if ((pw = rfSwitch.pulse(&high))){
		decoder.pulse(pw, high);
	}

	if (decoder.hasDetected()){
		data = decoder.getData();
		Serial.println(data.temp);
	}

}
