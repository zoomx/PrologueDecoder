#include "Arduino.h"
#include <PrologueDecoder.h>

const char SYNC_KEY[] = {1,0,0,1,1,0,0,1};
const int TWOPOWERS[] = {1,2,4,8,16,32,64,128,256,512,1024};

enum {IDLE,SYNCING,SYNCED};

byte nibbleToHex(byte * nibble){
	return (nibble[0]*8 + nibble[1]*4 + nibble[2]*2 + nibble[3]*1);
}


PrologueDecoder::PrologueDecoder(){
	reset();
	detected = false;
}

void PrologueDecoder::pulse(word width, bool high){

	switch (state){
	
	case IDLE:
		{
			if (high && ( (width > 330) && (width < 530))){
				// Might be the start of a sync bit
				state = SYNCING;
			}
		}
	case SYNCING:
		{
			if (!high && ( (width > 7500) && (width < 10000))){
				state = SYNCED;
			} else {
				reset();
			}
		}
	case SYNCED:
		{
			if ( high && !(halftime%2) ){
				// High part of bit
				if  ( (width < 330) || (width > 530) ){
					reset();
				}

			} else if ( !high && (halftime%2) ){
				// Low part of bit
				if ( (width > 1500) && (width < 2500)){
					// Short low: "0"
					bits[bitN++] = 0;
				} else if ( (width > 3500) && (width < 4500)){
					// Long low: "1"
					bits[bitN++] = 1;
				}else{
					/* Don't worry about it... */
				}

			} else {
				reset();
			}

			if (bitN == 36){
				detected = true;
				decode();
				reset();
			}

			halftime++;

		break;

		}
	}
}

bool PrologueDecoder::hasDetected(){
	if (detected){
		detected = false;
		return true;
	}
	return false;
}

Data PrologueDecoder::getData(){
	return data;
}

void PrologueDecoder::decode(){

	byte hex[9];
	for (int i = 0; i<36; i += 4){
		hex[i/4] = nibbleToHex(bits+i);
	}

	float sign = (bits[16]*(-2) + 1.0);
	float temp = 0.0;
	for (int i = 0; i < 11; i++){
		temp += bits[17+i]* TWOPOWERS[10 - i];
	}
	temp = sign*temp/10.0;

	data.ID = hex[0];
	data.rollingCode = hex[1]*16 + hex[2];
	data.channel = (bits[14]*2 + bits[15]) + 1;
	data.temp = temp;
}


void PrologueDecoder::reset(){
	state = IDLE;
	bitN = 0;
	halftime = 0;
	for (int i = 0; i < 36; i++){
		bits[i] = -1;
	}
}
