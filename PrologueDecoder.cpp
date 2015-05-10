/*
	The prologue protocol consists of 9 nibbles:
				0           1           2           3           4
		7654 3210   7654 3210   7654 3210   7654 3210   7654
		IIII RRRR   RRRR bBCC   TTTT TTTT   TTTT HHHH   HHHH

		I - ID
		R - Rolling ID
		b - battery
		B - Button
		C - channel
		T - Temp
		H - Humidity
*/

#include "Arduino.h"
#include <PrologueDecoder.h>

#define PROTOCOL_SIZE 36
#define setBit(byteArray,index) ((byteArray)[(index)/8] |= (1 << (7 - (index)%8)))

enum {IDLE,SYNCED};
const char SYNC_KEY[] = {1,0,0,1,1,0,0,1};

PrologueDecoder::PrologueDecoder(){
	reset();
}

bool PrologueDecoder::pulse(word width, bool high){

	if (high){
		if (width < 330 && width > 530){
			state = IDLE;
		}
		return false;
	}

	switch (state){
	
	case IDLE:

		if (width > 7500 && width < 10000){
			reset();
			state = SYNCED;
		}
		break;

	case SYNCED:

		// Zero bit
		if (width > 1500 && width < 2500){
			i++;
		}
		// One bit
		else if (width > 3500 && width < 4500){
			setBit(rawData,i);
			i++;
		}
		// Corrupt
		else{
			state = IDLE;
		}

		if (i == PROTOCOL_SIZE){
			decodeRawData();

			return true;
		}
		break;
	}

	return false;
}

PrologueData PrologueDecoder::getData(){
	return data;
}

void PrologueDecoder::decodeRawData(){
	data.ID = (rawData[0]&0xF0) >> 4;
	data.rollingID = ((rawData[0]&0x0F) << 4) | ((rawData[1]&0xF0) >> 4);
	data.battery = (rawData[1]&0x08) >> 3;
	data.button = (rawData[1]&0x04) >> 2;
	data.channel = (rawData[1]&0x03) + 1;
	data.temp = (((int16_t)(((uint16_t)rawData[2] << 8) | (rawData[3]&0xF0)))/16)*0.1;
	
}


void PrologueDecoder::reset(){
	memset(rawData, 0, sizeof(rawData));
	state = IDLE;
	i = 0;
}
