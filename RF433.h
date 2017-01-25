#include "arduino.h"
#include <string.h>
#include <math.h>

#ifndef RF433_h
	#define RF433_h
	
	class RF433
	{
		public:
		RF433(int RXPIN);
		bool data_OK;
	    //bool received;
		bool received();
		float getTemperature();
		void enableRF();
		void disableRF();
		private:
		char bitreceived;
		float bintodec(char  input[]);
		bool crc(char input[]);
	};
	
#endif

