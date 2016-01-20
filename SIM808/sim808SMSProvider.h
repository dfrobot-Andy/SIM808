#ifndef __SIM808SMSPROVIDER__
#define __SIM808SMSPROVIDER__

#include <sim808ModemCore.h>
#include <sim808MobileSMSProvider.h>

class SIM808SMSProvider :public SIM808MobileSMSProvider, public SIM808BaseProvider{
	public:
		SIM808SMSProvider();

		void manageResponse(int from, int to);

		inline int beginSMS(const char* to);

		inline void writeSMS(char c);

		inline int endSMS();

		int availableSMS();

		int peekSMS();

		void flushSMS();

		int remoteSMSNumber(char* number, int nlength); //Before reading the SMS, read the phone number.

		int readSMS();

		int ready(){
			SIM808ModemCore_t.manageReceivedData();
			return SIM808ModemCore_t.getCommandError(); //return SIM808BaseProvider::Ready();
		};

	private:
		int idSMS; // Id from current SMS being read.
		bool flagReadingSMS; // To detect first SMS char if not yet reading.
		bool fullBufferSMS; // To detect if the SMS being read needs another buffer. 
		bool twoSMSinBuffer; // To detect if the buffer has more than 1 SMS.
		bool checkSecondBuffer; // Pending to detect if the second buffer has more than 1 SMS.

		void beginSMSContinue();

		void endSMSContinue();

		void availableSMSContinue();

		void flushSMSContinue();
		//Ω‚ŒˆCMGLœÏ”¶
		bool parseCMGLAvailable(bool& rsp);
};





#endif