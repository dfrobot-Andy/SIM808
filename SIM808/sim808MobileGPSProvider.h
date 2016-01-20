#ifndef __SIM808MOBILEGPSPROVIDER_
#define __SIM808MOBILEGPSPROVIDER_

#include <debugDF.h>

enum SIM808_GPSStatus_st{ IDLE_GPS, GPSREV, NODATA };

class SIM808MobileGPSProvider{
	public:
		virtual inline void start(){};
		virtual inline void stop(){};
		virtual int ready() = 0;
		virtual void initialize(){};
};

extern SIM808MobileGPSProvider* SIM808MobileGPSProvider_p;


#endif




