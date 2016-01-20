#include <sim808SMSService.h>
#include <sim808MobileNetworkProvider.h>
#include <Arduino.h>
#include <sim808SMSProvider.h>
  
SIM808SMSProvider SIM808SMSProvider_t;

#define SIM808SMSSERVICE_SYNCH 0x01
#define __TOUT__	1000

SIM808SMSService::SIM808SMSService(bool synch){
	if (synch){
		flags |= SIM808SMSSERVICE_SYNCH;
	}
}

int SIM808SMSService::ready(){
	return SIM808MobileSMSProvider_p->ready();
}

int SIM808SMSService::beginSMS(const char* to){
	return waitForAnswer(SIM808MobileSMSProvider_p->beginSMS(to));
}

int SIM808SMSService::endSMS(){
	return waitForAnswer(SIM808MobileSMSProvider_p->endSMS());
}

size_t SIM808SMSService::write(uint8_t c){
	SIM808MobileSMSProvider_p->writeSMS(c);
}

void SIM808SMSService::flush(){
	SIM808MobileSMSProvider_p->flushSMS();
	waitForAnswer(1);
}

int SIM808SMSService::available(){
	return waitForAnswer(SIM808MobileSMSProvider_p->availableSMS());
}

int SIM808SMSService::remoteNumber(char* number, int nlength){
	return SIM808MobileSMSProvider_p->remoteSMSNumber(number,nlength);
}

int SIM808SMSService::read(){
	return SIM808MobileSMSProvider_p->readSMS();
}

int SIM808SMSService::peek(){
	return SIM808MobileSMSProvider_p->peekSMS();
}

int SIM808SMSService::waitForAnswer(int rVal){
	if (flags&SIM808SMSSERVICE_SYNCH){
		unsigned long m;
		m = millis();
		while (((millis() - m) < __TOUT__) && (ready() == 0)){
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




