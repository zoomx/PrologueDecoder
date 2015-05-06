
#include "Arduino.h"
#include <TempDecoder.h>



TempDecoder::TempDecoder(){
	reset();
	pReset();
}



byte TempDecoder::nibbleToHex(byte * nibble, bool reverse){
	if (reverse){
		return (nibble[0]*1 + nibble[1]*2 + nibble[2]*4 + nibble[3]*8);
	} else {
		return (nibble[0]*8 + nibble[1]*4 + nibble[2]*2 + nibble[3]*1);
	}

}

bool TempDecoder::interpretData(){
	byte hex[16];
	for (int i = 0; i<64; i += 4){
		hex[i/4] = nibbleToHex(bits+i,true);
	}
	
	sensorID = hex[0]*4096 + hex[1]*256 + hex[2]*16 + hex[3];
	sensorChannel = hex[4];
	sensorRollingCode = hex[5]*2 + hex[6];
	sensorFlag = hex[7];
	sensorTemp = (hex[10]*10.0 + hex[9]*1.0 + hex[8]*0.1);
	if (hex[11]){sensorTemp *= -1;}
	
}

bool TempDecoder::gotBit(byte bit){

	switch (state){
		
		case IDLE:
			if (bit && (halftime == 2)){
				// This is what's left of the first sync 1, encoded as 01
				syncOnes = 1;
				state = ONES;
			}
			break;
		case ONES:
			{
			bool odd = (halftime/2) % 2;
			if (   (bit && odd)  || (!bit && !odd) ) {
				syncOnes++;
			} else if ((syncOnes >= 30) && bit) {
				// We have synced.. maybe

				bitN++;
				state = SYNCING;
			} else {
				reset();
			}
			break;
			}
		case SYNCING:
			{
			if (bit != SYNC_KEY[bitN]){
				reset();
				return false;
			}
			bitN++;

			if (bitN == 8){
				state = RECV;
				bitN = 0;
			}

			break;
			}
		case RECV:
			{
			rawBits[bitN] = bit;
			bitN++;
			if (bitN == 128){

				// Convert to actual bits
				for (int i = 0; i < 64; i++){
					bits[i] = rawBits[2*i + 1];

				}

				interpretData();
				reset();
				return true;
			}
			
			break;
			}
	
	}
	return false;
}
bool TempDecoder::interpretPrologue(){


	byte hex[9];
	for (int i = 0; i<36; i += 4){
		hex[i/4] = nibbleToHex(pBits+i, false);
	}



	sensorID = hex[0];
	sensorRollingCode = hex[1]*16 + hex[2];
	sensorChannel = (pBits[14]*2 + pBits[15]) + 1;

	// temp, stored as 12 bit signed something
	float tempSign = (pBits[16]*2 + 1.0);
	sensorTemp = 0.0;
	for (int i = 0; i < 11; i++){
		sensorTemp += pBits[17+i]* TWOPOWERS[10 - i]/10.0;
	}
	sensorTemp *= tempSign;



	return true; 
}

bool TempDecoder::decodePrologueSensor(word width, byte high){

	switch (prologueState){
	
	case PROLOGUE_IDLE:
		{
			if (high && ( (width > 330) && (width < 530))){
				// Might be the start of a sync bit
				prologueState = PROLOGUE_SYNCING;
				return false;
			} else {
				return false;
			}
		}
	case PROLOGUE_SYNCING:
		{
			if (!high && ( (width > 7500) && (width < 10000))){
				prologueState = PROLOGUE_SYNCED;
				return false;
			} else {
				pReset();
				return false;
			}
		}
	case PROLOGUE_SYNCED:
		{
			// Receiving stuff
			if ( high && !(pHalftime%2) ){
				// High part of bit
				if  ( (width < 330) || (width > 530) ){
					pReset();
					return false;
				}

			} else if ( !high && (pHalftime%2) ){
				// Low part of bit
				if ( (width > 1500) && (width < 2500)){
					// Short low: "0"
					pBits[pBitN] = 0;
					pBitN++;
				} else if ( (width > 3500) && (width < 4500)){
					// Long low: "1"
					pBits[pBitN] = 1;
					pBitN++;
				}
			} else {
				// Error
				pReset();
				return false;
			}

			// Done yet?
			if ((pBitN == 36) && interpretPrologue()){
				return true;
			}

			pHalftime++;
		break;
		}
	}
	return false;

}

void TempDecoder::pReset(){
	prologueState = PROLOGUE_IDLE;
	pBitN = 0;
	pHalftime = 0;
	for (int i = 0; i < 36; i++){
		pBits[i] = -1;
	}
}

bool TempDecoder::pulse(word width, byte high){
	// Called when a transition has happend. "high" is the previous state, and width is the time in us of the length of that state
	//Serial.print("width: ");Serial.print(width);Serial.print("  ");

	// Prologue
	if (decodePrologueSensor(width,high)){
		// The prologue decoder has successfully decoded a message..
		return true;
	}

	// Determine if this is a long or short pulse
	switch (high){
	case 1:
		pulseType = wasHighFor(width);
		break;
	case 0:
		pulseType = wasLowFor(width);
		break;
	}


	// Increment the half time clock
	switch (pulseType){
	case 0:
		// Short pulse
		halftime += 1;
		break;
	case 1:
		// Long pulse
		halftime += 2;
		if (halftime%2){
			// clock boundary. Should not happen, as no change could have happend in the middle of this clock period.
			reset();
			return false;
		}

		break;
	default:
		reset();
		return false;
		break;
	}

	//Serial.print("C:");Serial.print(halftime);Serial.print(" ");

	// If we are in the middle of a clock period
	if (halftime%2 == 0){
		return gotBit(high);
	}
	return false;

	
}

int TempDecoder::wasHighFor(word width){

	if ((width >= 200) && (width < 615)){
		// Short
		return 0;
	} else if ((width >= 615) && (width < 1100)){
		// Long
		return 1;
	} else {
		//Error
		return -1;
	}
	
}

int TempDecoder::wasLowFor(word width){

	if ((width >= 400) && (width < 850)){
		// Short
		return 0;
	} else if ((width >= 850) && (width < 1400)){
		// Long
		return 1;
	} else {
		//Error
		return -1;
	}
	
}

void TempDecoder::reset(){
	// Reset
	state = IDLE;
	syncOnes = 0;
	bitN = 0;
	halftime = 0;

	for (int i = 0; i < 64; i++){
		rawBits[i] = -1;
	}

}