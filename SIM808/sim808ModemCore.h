#ifndef __SIM808MODEMCORE__
#define __SIM808MODEMCORE__

#include <sim808Timer2Serial.h>
#include <sim808BaseProvider.h>
#include <sim808mobileaccessprovider.h>
#include <Print.h>
#include <debugDF.h>

#define UMPROVIDERS	3

class SIM808ModemCore:public SIM808Timer2SerialMgr,public Print{
	private:
		// Phone number, used when calling, sending SMS and reading calling numbers
		// Also PIN in modem configuration
		// Also APN
		// Also remote server
		char* phoneNumber;

		// Working port. Port used in the ongoing command, while opening a server
		// Also for IP address length
		int port;

		// 0 = ongoing
		// 1 = OK
		// 2 = Error. Incorrect state
		// 3 = Unexpected modem message
		// 4 = OK but not available data. 
		uint8_t commandError;

		// Counts the steps by the command
		uint8_t commandCounter;

		// Presently ongoing command
		SIM808_CommandType ongoingCommand;

		// Enable/disable debug
		bool _debug;
		int _dataInBufferFrom;
		int _dataInBufferTo;

		SIM800_NetworkStatus_t _status;
		
		SIM808BaseProvider* UMProvider[UMPROVIDERS];
		

		void manageMsgNow(int from, int to);
		unsigned long milliseconds;
	public:
		SIM808ModemCore();	//constructor
		SIM808Timer2Serial gss; //Direct access to modem
		char* getPhoneNumber(){ return phoneNumber; };
		void setPhoneNumber(char* nbr){ phoneNumber = nbr; };
		int getPort(){ return port; };
		void setPort(int p){ port = p; };
		uint8_t getCommandError(){ return commandError; };
		void setCommandError(uint8_t n){ commandError = n; };
		uint8_t getCommandCounter(){ return commandCounter; };
		void setCommandCounter(uint8_t c){ commandCounter = c; };
		void setOngoingCommand(SIM808_CommandType c){ ongoingCommand = c; };
		SIM808_CommandType getOngoingCommand(){ return ongoingCommand; };
		void openCommand(SIM808BaseProvider* activeProvider, SIM808_CommandType c);
		void closeCommand(int code);
		size_t write(uint8_t c);

		virtual size_t writePGM(/*PROGMEM prog_char*/PGM_P str, bool CR = true);
		void setDebug(bool db){ _debug = db; };
		bool genericParse_rsp(bool& rsp, char* string = 0, char* string2 = 0);//Generic response parser
		bool genericParse_rsp(bool& rsp, char* string, uint8_t slen);
		void genericCommand_rq(/*PROGMEM prog_char*/PGM_P str, bool addCR = true);//Generates a generic AT command request from PROGMEM prog_char buffer
		void genericCommand_rqc(const char* str, bool addCR = true);
		inline SIM808CircularBuffer& theBuffer(){ return gss.cb; };
		inline void setStatus(SIM800_NetworkStatus_t status){ _status = status; };
		inline SIM800_NetworkStatus_t getStatus(){ return _status; };
		void registerUMProvider(SIM808BaseProvider* provider);//
		void unRegisterUMProvider(SIM808BaseProvider* provider);//
		void registerActiveProvider(SIM808BaseProvider* provider){ activeProvider = provider; };
		
		void manageMsg(int from, int to);//
		void manageReceivedData();
		unsigned long takeMilliseconds();
		void  delayInsideInterrupt(unsigned long milliseconds);

		SIM808BaseProvider* activeProvider;
};

extern SIM808ModemCore SIM808ModemCore_t;

#endif
