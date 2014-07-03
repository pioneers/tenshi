#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SMART_ID_LEN 8
typedef int SSState;


// to be put in a header file
typedef struct
{
	SSState *arr;
	size_t len;
} KnownIDs;

void addIDToArray(KnownIDs *sensorIDs, uint8_t ID[SMART_ID_LEN]);
void enumeration(KnownIDs *sensorIDs);
void findSensorID(KnownIDs *sensorIDs, uint8_t currID[SMART_ID_LEN],  uint8_t bitDepth);
void getToState(uint8_t currID[SMART_ID_LEN], uint8_t bitDepth);
uint8_t getBit(uint8_t currID[SMART_ID_LEN], uint8_t pos);
int isSomethingThere();

// functions
void addIDToArray(KnownIDs *sensorIDs, uint8_t ID[SMART_ID_LEN])
{

}


void enumeration(KnownIDs *sensorIDs)
{
	findSensorID(sensorIDs, 0, 0);
}


int isSomethingThere()
{
	return 0;
}

void findSensorID(KnownIDs *sensorIDs, uint8_t currID[SMART_ID_LEN],  uint8_t bitDepth)
{
	getToState(currID, bitDepth); //010110101 , 3
	
	if  (!isSomethingThere())
	{
		return;
	}
	
	if (bitDepth == SMART_ID_LEN)
	{
		addIDToArray(sensorIDs, currID);
		return;
	}
	currID[bitDepth/8] |= (1 << bitDepth % 8);
	findSensorID(sensorIDs, currID, bitDepth + 1);
	currID[bitDepth/8] &= ~(1 << bitDepth % 8);
	findSensorID(sensorIDs, currID, bitDepth + 1);
}



void getToState(uint8_t currID[SMART_ID_LEN], uint8_t bitDepth)
{	
	//ss_send_enum_reset();// ADD ARGUMENT); 
	uint8_t pos = 0;
	uint8_t nextBit = getBit(currID, pos); // the next bit in id that we will include
	

	//turnOff( nextBit^1, 0); // Casey: replace with whatever
	
	while (pos < bitDepth)
	{
		pos = pos + 1;
		nextBit = getBit(currID, pos);
		uint8_t notNextBit = nextBit ^ 1;
		//turnOff(notNextBit, pos); // Casey: turnOff function turn of bit at this position
	}
}


uint8_t getBit(uint8_t currID[SMART_ID_LEN], uint8_t pos)
{
	return (currID[pos/8] & 1 << (pos % 8)) >> pos % 8;
}