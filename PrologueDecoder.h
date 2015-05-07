#ifndef PrologueDecoder_h
#define PrologueDecoder_h

#include "Arduino.h"


typedef struct {
	word ID;
	byte channel;
	byte rollingCode;
	byte flag;
	float temp;
} Data;

class PrologueDecoder {

public:
	PrologueDecoder();
	void pulse(word width, bool high);
	bool hasDetected();
	Data getData();

private:
	Data data;
	void decode();
	void reset();
	bool detected;
	byte state;
	byte bits[36];
	byte bitN;
};


#endif
