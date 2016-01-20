#include <sim808VoiceCallService.h>
#include <Arduino.h>
#include <sim808VoiceProvider.h>

SIM808VoiceProvider SIM808VoiceProvider_t;//

#define SIM808VOICECALLSERVICE_SYNCH 0x01
#define __TOUT__	10000

SIM808VoiceCallService::SIM808VoiceCallService(bool synch){
	if (synch){
		flags |= SIM808VOICECALLSERVICE_SYNCH;
	}
	SIM808MobileVoiceProvider_t->initialize();//
}

SIM808_VoiceCall_st SIM808VoiceCallService::getvoiceCallStatus(){
	if (SIM808MobileVoiceProvider_t == 0){
		return IDLE_CALL;
	}
	return SIM808MobileVoiceProvider_t->getvoiceCallStatus();
}

int SIM808VoiceCallService::ready(){
	if (SIM808MobileVoiceProvider_t == 0){
		return 0;
	}
	return SIM808MobileVoiceProvider_t->ready();
}

int SIM808VoiceCallService::voiceCall(const char* to, unsigned long timeout){
	if (SIM808MobileVoiceProvider_t == 0){
		return 0;
	}
	if (flags&SIM808VOICECALLSERVICE_SYNCH){
		SIM808MobileVoiceProvider_t->voiceCall(to);
		unsigned long m;
		m = millis();
		while (((millis() - m) < timeout) && (getvoiceCallStatus() == CALLING)){
			delay(200);//
		}
		if (getvoiceCallStatus() == TALKING){
			return 1;
		}else{
#if ANDYDEBUGx
		Serial.println("Voice Calling do not back data...");
#endif
		return 0;
		}
	}else{
		return SIM808MobileVoiceProvider_t->voiceCall(to);
	}
}

int SIM808VoiceCallService::answerCall(){
	if (SIM808MobileVoiceProvider_t == 0)
		return 0;
	return waitForAnswer(SIM808MobileVoiceProvider_t->answerCall());
}

int SIM808VoiceCallService::hangCall(){
	if (SIM808MobileVoiceProvider_t == 0)
		return 0;
	return waitForAnswer(SIM808MobileVoiceProvider_t->hangCall());
}

int SIM808VoiceCallService::retrieveCallingNumber(char* buffer, int bufsize){
	if (SIM808MobileVoiceProvider_t == 0)
		return 0;
	return waitForAnswer(SIM808MobileVoiceProvider_t->retrieveCallingNumber(buffer,bufsize));
}

int SIM808VoiceCallService::waitForAnswer(int returnvalue){
	if (flags&SIM808VOICECALLSERVICE_SYNCH){
		unsigned long m;
		m = millis();
		while (((millis() - m) < __TOUT__) && (ready() == 0))
			delay(100);
		if (ready() == 1)
			return 1;
		else
			return 0;
	}
	return ready();
}



