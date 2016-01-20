#ifndef __SIM808TCPSERVICE__
#define __SIM808TCPSERVICE__

#include <Stream.h>
#include <sim808mobileaccessprovider.h>
#include <sim808MobileTCPProvider.h>

class SIM808TCPService{
	public:
		SIM808TCPService();

		bool connectTCP(char *addr);//
		bool disConnectTCP();
		bool sendTCPdata(char *dta);//
	//	bool sendTCPdata(char *dta, char len, bool rep);
		bool setServerParameters(char* idName, char* idDev, char* pKey);

	private:
		int flags;
		void restart(void);
		
};


#endif


