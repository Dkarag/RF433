
#include "arduino.h"
#include <string.h>
#include <math.h>
#include "RF433.h"
bool _received = false;
// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances
int RXPIN;
//received = false;
int RING_BUFFER_SIZE=1024;
unsigned long timings[1024];
unsigned int syncIndex1 = 0;  // index of the first sync signal
unsigned int syncIndex2 = 0;  // index of the second sync signal
int SYNC_LENGTH=9000;
int SEP_LENGTH=500;
int BIT1_LENGTH=4000;
int BIT0_LENGTH=2000;
int dev=1300;
RF433::RF433(int PIN) // sets the RX pin
{
    RXPIN = PIN;
}

float RF433::bintodec(char input[]) {
    float value = 666;
	int i = 12; 
	if(input[i]=='1'&&input[i+1]=='1'&&input[i+2]=='1'&&input[i+3]=='1')
	{ 	//negative number
		value = 0;
		for (int i = 16; i < 24; i++) {
			if (input[i] == '1') {
				value = value + pow(2, 23 - i);
			}
		}
		value=value*(-1);
	}
	else
	{	value = 0;
		for (int i = 15; i < 24; i++) {
			if (input[i] == '1') {
				value = value + pow(2, 23 - i);
			}
		}
	}
	return value / 10;
}

bool RF433::received()
{
	return _received;
}
bool RF433::crc(char input[]) {
	char x;
	int checksumcalc = 0;
	for (int i = 30; i > -1; i--) {
		x = input[i];
		if (x == '1') {
			checksumcalc = checksumcalc ^ 1;
		} else
		checksumcalc = checksumcalc ^ 0;
	}
	x = input[31];
	int y = x - '0'; //convert char to string
	if (y == checksumcalc)
	return true;
    else
	return false;
}

// detect if a sync signal is present
bool isSync(unsigned int idx) {
	unsigned long t0 = timings[(idx+RING_BUFFER_SIZE-1) % RING_BUFFER_SIZE];
	unsigned long t1 = timings[idx];
	// on the temperature sensor, the sync signal
	// is roughtly 9.0ms. Accounting for error
	// it should be within 8.0ms and 10.0ms
	if (t0>(SEP_LENGTH-100) && t0<(SEP_LENGTH+100) &&
    t1>(SYNC_LENGTH-dev) && t1<(SYNC_LENGTH+dev) &&
    digitalRead(RXPIN) == HIGH) {
		return true;
	}
	return false;
}

/* Interrupt 1 handler */
void handler() {
	static unsigned long duration = 0;
	static unsigned long lastTime = 0;
	static unsigned int ringIndex = 0;
	static unsigned int syncCount = 0;
	// ignore if we haven't processed the previous received signal
	if (_received == true) {
		return;
	}
	// calculating timing since last change
	long time = micros();
	duration = time - lastTime;
	lastTime = time;
	
	// store data in ring buffer
	ringIndex = (ringIndex + 1) % RING_BUFFER_SIZE;
	timings[ringIndex] = duration;
	
	// detect sync signal
	if (isSync(ringIndex)) {
		syncCount ++;
		// first time sync is seen, record buffer index
		if (syncCount == 1) {
			syncIndex1 = (ringIndex+1) % RING_BUFFER_SIZE;
		} 
		else if (syncCount == 2) {
			// second time sync is seen, start bit conversion
			syncCount = 0;
			syncIndex2 = (ringIndex+1) % RING_BUFFER_SIZE;
			unsigned int changeCount = (syncIndex2 < syncIndex1) ? (syncIndex2+RING_BUFFER_SIZE - syncIndex1) : (syncIndex2 - syncIndex1);
			// changeCount must be 66 -- 32 bits x 2 + 2 for sync
			//Serial.print("changeCount=");
			//Serial.println(changeCount);
			if (changeCount != 66) {
				_received = false;
				syncIndex1 = 0;
				syncIndex2 = 0;
			} 
			else {
				_received = true;		
			}
		}
	}
}

float RF433::getTemperature()
{
	char Str[32];
	int k=0;
	char bitreceived;
	// disable interrupt to avoid new data corrupting the buffer
	//detachInterrupt(digitalPinToInterrupt(RXPIN));
	// loop over buffer data
	for(unsigned int i=syncIndex1; i!=syncIndex2; i=(i+2)%RING_BUFFER_SIZE) {
		unsigned long t0 = timings[i], t1 = timings[(i+1)%RING_BUFFER_SIZE];
		if (t0>(SEP_LENGTH-100) && t0<(SEP_LENGTH+100)) {
			if (t1>(BIT1_LENGTH-dev) && t1<(BIT1_LENGTH+dev)) {
				//Serial.print("1");
				bitreceived='1';
				} else if (t1>(BIT0_LENGTH-dev) && t1<(BIT0_LENGTH+dev)) {
				//Serial.print("0");
				bitreceived='0';
				} else {
				//Serial.print("SYNC");  // sync signal
			}
			} else {
			//Serial.print("?");  // undefined timing
			bitreceived='?';
		}
		Str[k]=bitreceived;
		k++;
	}
   // String stringOne(Str);
	//String temp_out=stringOne.substring(16, 24);
	//Serial.println(temp_out);
	
	//Serial.print(RF433::bintodec(temp_out));
	// delay for 1 second to avoid repetitions
	//delay(1000);
	data_OK = true;
	for (int i = 12; i < 24; i++) {
		char x = Str[i];
		if (x == '?') {
			data_OK = false;
		} 
	}
	/*
		if (!RF433::crc(Str))
		{
		data_OK = false;
		}
	*/
	_received = false;
	syncIndex1 = 0;
	syncIndex2 = 0;		
	// re-enable interrupt
	float temp=bintodec(Str);
	//  attachInterrupt(digitalPinToInterrupt(RXPIN), handler, CHANGE);
	
	return temp;
	
}

void RF433::enableRF()
{
	attachInterrupt(digitalPinToInterrupt(RXPIN), handler, CHANGE);
}

void RF433::disableRF()
{
	// disable interrupt to avoid new data corrupting the buffer
	detachInterrupt(digitalPinToInterrupt(RXPIN));
	}
	
		