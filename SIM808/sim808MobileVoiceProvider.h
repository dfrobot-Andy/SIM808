#ifndef __SIM808MOBILEVOICEPROVIDER__
#define __SIM808MOBILEVOICEPROVIDER__

#include <debugDF.h>

enum SIM808_VoiceCall_st{ IDLE_CALL, CALLING, RECEIVINGCALL, TALKING };

class SIM808MobileVoiceProvider{
	public:
		//Initialize the object
		virtual void initialize(){};
		// Launch a voice call
		virtual int voiceCall(const char* number) = 0;
		virtual int answerCall() = 0;
		virtual int hangCall() = 0;
		//Retrieve phone number of caller
		virtual int retrieveCallingNumber(char* buffer, int bufsize) = 0;
		//Returns voice call status
		virtual SIM808_VoiceCall_st getvoiceCallStatus() = 0;
		//Set voice call status
		virtual void setvoiceCallStatus(SIM808_VoiceCall_st status) = 0;
		//Get last command status
		virtual int ready() = 0;
};



extern SIM808MobileVoiceProvider* SIM808MobileVoiceProvider_t;



#endif