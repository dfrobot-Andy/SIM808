#ifndef __SIM808MOBILEACCESSPROVIDER__
#define __SIM808MOBILEACCESSPROVIDER__

#include <debugDF.h>

enum SIM800_NetworkStatus_t{
	ERROR,
	IDLE,
	CONNECTING,
	SIM808_READY,
	GPRS_READY,
	GPS_READY,
	TCP_CONNECTED,
	TCP_DISCONNECTED,
	TRANSPARENT_CONNECTED
};

class SIM800MobileAccessProvider{
	public:
		virtual inline SIM800_NetworkStatus_t begin(char* pin=0, bool restart=true, bool synchronous=true) = 0;
		//Check network access status
		virtual inline int isAccessAlive() = 0; 
		virtual inline bool ShutDown() = 0;
		virtual int ready() = 0;
};

#endif