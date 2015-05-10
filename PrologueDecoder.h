#ifndef PrologueDecoder_h
#define PrologueDecoder_h

#include "Arduino.h"

typedef struct {
	byte ID : 4;
	byte rollingID : 8;
	bool battery : 1;
	bool button : 1;
	byte channel : 2;
	float temp;
} PrologueData;

class PrologueDecoder {

public:
	PrologueDecoder();
	bool pulse(word width, bool high);
	PrologueData getData();

private:
	void decodeRawData();
	void reset();
	byte state;
	byte rawData[5], i;
	PrologueData data;
};


#endif
