#ifndef __SIM808MOBILESMSPROVIDER__
#define __SIM808MOBILESMSPROVIDER__


class SIM808MobileSMSProvider{
	public:
		virtual inline int beginSMS(const char* to){ return 0; };

		virtual inline void writeSMS(const char c){};

		virtual inline int endSMS(){ return 0; };

		virtual inline int availableSMS(){ return 0; };

		virtual inline int peekSMS(){ return 0; };

		virtual inline void flushSMS(){ return; };
		//
		virtual inline int remoteSMSNumber(char* number, int nlength){ return 0; };

		virtual inline int readSMS(){ return 0; };

		virtual int ready() = 0;
};

extern SIM808MobileSMSProvider* SIM808MobileSMSProvider_p;


#endif