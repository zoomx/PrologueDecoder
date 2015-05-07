#include <RF_Switch.h>
#include <PrologueDecoder.h>

PrologueDecoder decoder;
RF_Switch rfSwitch(2);

int pw;
Data data;
void setup(){
	pinMode(2,INPUT);
	Serial.begin(115200);
	Serial.println("Connected");
}

void loop(){
	if ((pw = rfSwitch.highPulse())){
		decoder.pulse(pw, 1);
	}

	if ((pw = rfSwitch.lowPulse())){
		decoder.pulse(pw, 0);
	}

	if (decoder.hasDetected()){
		data = decoder.getData();
		Serial.println(data.temp);
	}

}
