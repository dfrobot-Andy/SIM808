#ifndef __SIM808VOICEPROVIDER__
#define __SIM808VOICEPROVIDER__

#include <sim808MobileVoiceProvider.h>
#include <sim808ModemCore.h>
#include <sim808BaseProvider.h>
#include <debugDF.h>

class SIM808VoiceProvider :public SIM808MobileVoiceProvider, public SIM808BaseProvider{
	public:
		SIM808VoiceProvider();
		void initialize();
		void manageResponse(int from, int to);
		int voiceCall(const char* number);
		int answerCall();
		int hangCall();
		int retrieveCallingNumber(char* buffer, int bufsize);
		int ready(){
			SIM808ModemCore_t.manageReceivedData();
			return SIM808ModemCore_t.getCommandError(); //return SIM808BaseProvider::Ready();
		};

		bool recognizeUnsolicitedEvent(int oldTail);

		SIM808_VoiceCall_st getvoiceCallStatus(){ ready(); return _voiceCallstatus; };

		void setvoiceCallStatus(SIM808_VoiceCall_st status) { _voiceCallstatus = status; };


	private:
		int phonelength; // Phone number length

		SIM808_VoiceCall_st _voiceCallstatus; // The voiceCall status

		void voiceCallContinue();

		void answerCallContinue();

		void hangCallContinue();

		void retrieveCallingNumberContinue();

		bool parseCLCC(char* number, int nlength);
};


#endif