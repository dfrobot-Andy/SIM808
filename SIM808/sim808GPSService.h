#ifndef __SIM808GPSSERVICE__
#define __SIM808GPSSERVICE__
#include <Stream.h>
#include <sim808mobileaccessprovider.h>
#include <sim808MobileGPSProvider.h>

class SIM808GPSService{
	private:
		uint8_t flags;
		int waitForAnswer(int reValue);
		SIM800_NetworkStatus_t begin(void);
		//SIM808_GPS_st gps_status;
		int ready();

	public:
		SIM808GPSService(bool synch=true);
		bool readGPS(char* str);
		//SIM808_GPS_st getGPSStatus(){ return gps_status; };
		//void setGPSStatus(SIM808_GPS_st st){ gps_status = st; };
		void restart();
		bool availableGPS();
		void stopdisplay();
		bool stop();
};





#endif







