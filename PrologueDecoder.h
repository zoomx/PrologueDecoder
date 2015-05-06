#ifndef TempDecoder_h
#define TempDecoder_h

#include "Arduino.h"

const char SYNC_KEY[] = {1,0,0,1,1,0,0,1};
const int TWOPOWERS[] = {1,2,4,8,16,32,64,128,256,512,1024};

class TempDecoder {

public:
	TempDecoder();

	// Methods
	bool pulse(word width, byte high);


	// Sensor data
	word sensorID;
	byte sensorChannel;
	byte sensorRollingCode;
	byte sensorFlag;
	float sensorTemp;

private:
	
	// Prologue
	bool decodePrologueSensor(word width, byte high);
	byte prologueState;
	byte prologueBitN;
	enum {PROLOGUE_IDLE,PROLOGUE_SYNCING, PROLOGUE_SYNCED};
	byte pBits[36];
	byte pBitN;
	int pHalftime;
	void pReset();
	bool interpretPrologue();

	// General
	byte nibbleToHex(byte * nibble, bool reverse);
	

};


#endif
