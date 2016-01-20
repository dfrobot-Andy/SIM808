#ifndef __SIM808GPSPROVIDER__
#define __SIM808GPSPROVIDER__

#include <sim808MobileGPSProvider.h>
#include <sim808ModemCore.h>
#include <sim808BaseProvider.h>
#include <debugDF.h>

class SIM808GPSProvider :public SIM808MobileGPSProvider, public SIM808BaseProvider{
	public:
		SIM808GPSProvider();
		void initialize(void);
		void manageResponse(int from, int to);//
		int ready(void){
			SIM808ModemCore_t.manageReceivedData();
			return SIM808ModemCore_t.getCommandError(); //return SIM808BaseProvider::Ready();
		};
		void dataProcessing(void);//
		bool availableGPS(void);//
		bool getGPSdata(char *str);
		bool recognizeUnsolicitedEvent(int oldTail);
		void beginModemConfigurationCB(void);
		void endModemConfigurationCB(void);
		void start(void);
		void stop(void);//
		void stopGPSPWD(void);//

		void setGPSStatus(SIM808_GPSStatus_st st){ gpsStatus_st = st; };
		SIM808_GPSStatus_st getGPSStatus(void){ return gpsStatus_st; };

		char gpsRep[100];
		//andy_gps gps_t;
	private:
		SIM808_GPSStatus_st gpsStatus_st;
		char* gpsDta;//
		bool retrieveGPSdta(char* buffer, int bufsize);
		bool locateString(const char* reference, const char* original, char* from);
		bool locateString(const char* reference, const char* original);
};


#endif


