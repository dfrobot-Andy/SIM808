#ifndef __SIM808MOBILETCPPROVIDER__
#define __SIM808MOBILETCPPROVIDER__

#include <debugDF.h>

enum SIM808_TCPStatus_st{ IDLE_TCP, LAUNCHED, CONNECTED, DISCONNECTED,SETTING };

class SIM808MobileTCPProvider{
public:

	virtual int ready() = 0;
	virtual void initialize(){};
};

extern SIM808MobileTCPProvider* SIM808MobileTCPProvider_p;




#endif




