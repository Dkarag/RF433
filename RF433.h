/*
	Test.h - Test library for Wiring - description
	Copyright (c) 2006 John Doe.  All right reserved.
*/
// ensure this library description is only included once


#include "arduino.h"
#include <string.h>
#include <math.h>

#ifndef RF433_h
	#define RF433_h
	
	
	// include types & constants of Wiring core API
	
	// library interface description
	class RF433
	{
		// user-accessible "public" interface
		public:
		RF433(int RXPIN);
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

