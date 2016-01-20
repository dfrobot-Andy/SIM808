#include <sim808GPSService.h>
#include <sim808MobileGPSProvider.h>
#include <sim808GPSProvider.h>
#include <Arduino.h>
#include <sim808AccessProvider.h>

#define SIM808GPSSERVICE_SYNCH 0x01
#define __TIMEOUT__	1000


SIM808GPSProvider SIM808GPSProvider_t;

SIM808GPSService::SIM808GPSService(bool synch){
	if (synch){
		flags = SIM808GPSSERVICE_SYNCH;	
	}
	SIM808MobileGPSProvider_p->initialize();
}

void SIM808GPSService::restart(){
	boolean notConnected = true;
	SIM808AccessProvider restartdebug;

	while (notConnected){
		if (restartdebug.begin("") == SIM808_READY){
			notConnected = false;
		}else{
			Serial.println("Not connected");
			delay(1000);
		}
	}
}

int SIM808GPSService::waitForAnswer(int reValue){
	if (flags&SIM808GPSSERVICE_SYNCH){
		unsigned long m;
		m = millis();
		while (((millis() - m) < __TIMEOUT__) && (ready() == 0)){
			delay(100);
		}
		if (ready() == 1){
			return 1;
		}else{
			return 0;
		}
	}
	return ready();
}

SIM800_NetworkStatus_t SIM808GPSService::begin(){
	//this->setGPSStatus(STARTGPS);
	SIM808MobileGPSProvider_p->start();//
	
	uint8_t i = 0;
	while (ready()==0){
		delay(1000);
		Serial.print("ready()...");
		Serial.println(i);
		i++;
		if (i > 5){
			i = 0;
			restart();//
			break;
		}
	}
	return SIM808ModemCore_t.getStatus();
}

bool SIM808GPSService::availableGPS(){//
	this->begin();
	this->stopdisplay();
	//this->setGPSStatus(NULLDATA);

	return (SIM808GPSProvider_t.availableGPS());
}

int SIM808GPSService::ready(){
	return SIM808GPSProvider_t.ready();
}

bool SIM808GPSService::readGPS(char *str){
	return (SIM808GPSProvider_t.getGPSdata(str));
}

//disenable GPS display
void SIM808GPSService::stopdisplay(){
	SIM808GPSProvider_t.stop();
}

//close GPS power
bool SIM808GPSService::stop(){
	SIM808GPSProvider_t.stopGPSPWD();
	return false;
}







