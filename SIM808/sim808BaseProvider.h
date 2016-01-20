#ifndef __SIM808BASEPROVIDER__
#define __SIM808BASEPROVIDER__

#include <sim808Timer2Serial.h>
#include <debugDF.h>
#include <avr/pgmspace.h>

enum SIM808_CommandType{
	XON, 
	NONE, 
	MODEMCONFIG, 
	ALIVETEST, 
	BEGINSMS, //4
	ENDSMS, 
	BEGINGPS,
	ENDGPS,
	BEGINGPRS,
	ENDGPRS,
	CHECKPOWER,
	AVAILABLESMS, 
	FLUSHSMS,
	VOICECALL,
	ANSWERCALL,
	HANGCALL,
	RETRIEVECALLINGNUMBER,
	ATTACHGPRS, 
	DETACHGPRS,
	CONNECTTCPCLIENT, //19
	DISCONNECTTCP,
	CONNECTTCP,
	TCPTRANSPORT
};

class SIM808BaseProvider{
	public:
		//Get last command status
		//auxLocate Buffer where to locate strings-->char _command_AT[] PROGMEM = "AT";
		void prepareAuxLocate(/*PROGMEM prog_char*/PGM_P str, char auxLocate[]);
		//Manages modem response
		virtual void manageResponse(int from, int to);
		//Recognize URC
		virtual bool recognizeUnsolicitedEvent(int from){ return false; };
};

#endif











