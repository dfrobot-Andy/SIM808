#ifndef __SIM808TCPPROVIDER__
#define __SIM808TCPPROVIDER__
#include <sim808MobileTCPProvider.h>
#include <sim808ModemCore.h>
#include <sim808BaseProvider.h>
#include <debugDF.h>

#define GPSDATAREPORT 1

class SIM808TCPProvider :public SIM808MobileTCPProvider, public SIM808BaseProvider{
	public:
		SIM808TCPProvider();
		//SIM800_NetworkStatus_t getTCPstatus(){ return SIM808ModemCore_t.getStatus(); };
		char reStr_t;
		char *tcpip;
		int ready(void){
			SIM808ModemCore_t.manageReceivedData();
			return SIM808ModemCore_t.getCommandError(); //return SIM808BaseProvider::Ready();
		};
		bool connectTCP(char *addr);//
		bool setServerParameters(char* idName, char* idDev, char* pKey);
		bool sendTCPdata(char *dta);//
		//bool sendTCPdata(char *dta, char strlen, bool rep);
		bool disConnectTCP();
		void delayInternal();
		void manageResponse(int from, int to);//
		void TCPModemConfigurationCB(void);
		void TCPTransmissionCB(void);
		bool recognizeUnsolicitedEvent(int oldTail);
		void initialize();

		//tcp
		SIM808_TCPStatus_st getTCPStatus(void){ return tcpStatus_t; };
		void setTCPStatus(SIM808_TCPStatus_st  st){ tcpStatus_t = st; };

	private:
		bool waitForAnswer(int reValue);
		char *pdta;
		char *idNames;
		char *idDevice;
		char *pKeys;
		bool gpsRep;
		char tcpLen;
		unsigned long n;
		unsigned long TCPSendTime(void);
		SIM808_TCPStatus_st tcpStatus_t;
};


#endif


