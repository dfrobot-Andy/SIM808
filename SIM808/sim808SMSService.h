#ifndef __SIM808SMSERVICE__
#define __SIM808SMSERVICE__

#include <sim808MobileSMSProvider.h>
#include <Stream.h>

class SIM808SMSService :public Stream{
	private:
		uint8_t flags;
		int waitForAnswer(int rVal);
	public:
		SIM808SMSService(bool synch=true);
		//
		size_t write(uint8_t c);
		//
		int beginSMS(const char* to);
		int ready();
		int endSMS();
		int available();
		int remoteNumber(char* number,int nlength);
		int read();
		int peek();
		void flush();
};




#endif