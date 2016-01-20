#ifndef __SIM808VOICECALLSERVICE__
#define __SIM808VOICECALLSERVICE__

#include <sim808MobileNetworkProvider.h>
#include <sim808MobileVoiceProvider.h>
#include <debugDF.h>


class SIM808VoiceCallService{
	private:
		uint8_t flags;
		int waitForAnswer(int reValue);
	public:
		SIM808VoiceCallService(bool synch=true);
		SIM808_VoiceCall_st getvoiceCallStatus();
		int ready();
		int voiceCall(const char* to,unsigned long timeout=30000);
		int answerCall();
		int hangCall();
		int retrieveCallingNumber(char* buffer,int bufsize);
};


#endif