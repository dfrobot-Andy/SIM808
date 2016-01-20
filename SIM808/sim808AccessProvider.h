#ifndef __SIM808ACCESSPROVIDER__
#define __SIM808ACCESSPROVIDER__

#include <sim808mobileaccessprovider.h>
#include <sim808ModemCore.h>
#include <sim808BaseProvider.h>
#include <debugDF.h>

class  SIM808AccessProvider :/*public SIM800MobileAccessProvider, */public SIM808BaseProvider{
	private:
		//Initialize main modem configuration
		int ModemConfiguration(char* pin);
		//Continue to modem configuration function
		void ModemConfigurationContinue();
		void isModemAliveContinue();
		bool flags_t;
	//	bool recognizeUnsolicitedEvent(int from);
	public:
		SIM808AccessProvider(bool debug=false);
		//~SIM808AccessProvider();
		SIM800_NetworkStatus_t begin(char* pin=0, bool restart=true, bool synchronous=true);

		int isAccessAlive();

		bool shutdown();
		int ready(){
			SIM808ModemCore_t.manageReceivedData();
			return SIM808ModemCore_t.getCommandError(); //return SIM808BaseProvider::Ready();
		};
		//Returns modem status
		inline SIM800_NetworkStatus_t getStatus(){ return SIM808ModemCore_t.getStatus(); };

		void manageResponse(int from, int to);

		int HWrestart();

		int HWstart();
};











#endif