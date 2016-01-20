#include <sim808VoiceProvider.h>
#include <Arduino.h>

SIM808VoiceProvider::SIM808VoiceProvider(){
	phonelength = 0;
	SIM808MobileVoiceProvider_t= this;
}

void SIM808VoiceProvider::initialize(){
	SIM808ModemCore_t.registerUMProvider(this);
}

//Voice Call main function.
int SIM808VoiceProvider::voiceCall(const char* to){
	SIM808ModemCore_t.genericCommand_rq(PSTR("ATD"), false);
	SIM808ModemCore_t.print(to);
	SIM808ModemCore_t.print(";\r\n");
	setvoiceCallStatus(CALLING);
	return 1;
}

//Retrieve calling number main function.
int SIM808VoiceProvider::retrieveCallingNumber(char* buffer, int bufsize){
	SIM808ModemCore_t.setPhoneNumber(buffer);
	phonelength = bufsize;
	SIM808ModemCore_t.setCommandError(0);
	SIM808ModemCore_t.setCommandCounter(1);
	SIM808ModemCore_t.openCommand(this, RETRIEVECALLINGNUMBER);
	retrieveCallingNumberContinue();
	return SIM808ModemCore_t.getCommandError();
}

//Retrieve calling number Continue function.
void SIM808VoiceProvider::retrieveCallingNumberContinue(){
	// 1:  AT+CLCC
	// 2: Receive +CLCC: 1,1,4,0,0,"num",129,""
	// This implementation really does not care much if the modem aswers trash to CMGL
	bool resp;
	//int msglength_aux;
	switch (SIM808ModemCore_t.getCommandCounter()) {
	case 1:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CLCC"));
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		if (parseCLCC(SIM808ModemCore_t.getPhoneNumber(), phonelength)){
			SIM808ModemCore_t.closeCommand(1);
		}
		break;
	}
}

//CLCC parse.	
bool SIM808VoiceProvider::parseCLCC(char* number, int nlength){
	SIM808ModemCore_t.theBuffer().extractSubString("+CLCC: 1,1,4,0,0,\"", "\"", number, nlength);
	SIM808ModemCore_t.theBuffer().Flush();
	return true;
}

//Answer Call main function.
int SIM808VoiceProvider::answerCall(){
	SIM808ModemCore_t.setCommandError(0);
	SIM808ModemCore_t.setCommandCounter(1);
	SIM808ModemCore_t.openCommand(this, ANSWERCALL);
	answerCallContinue();
	return SIM808ModemCore_t.getCommandError();
}

//Answer Call continue function.
void SIM808VoiceProvider::answerCallContinue(){
	// 1: ATA
	// 2: Waiting for OK

	// This implementation really does not care much if the modem aswers trash to CMGL
	bool resp;
	switch (SIM808ModemCore_t.getCommandCounter()) {
	case 1:
		// ATA ;
		SIM808ModemCore_t.genericCommand_rq(PSTR("ATA"));
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			setvoiceCallStatus(TALKING);
			if (resp) SIM808ModemCore_t.closeCommand(1);
			else SIM808ModemCore_t.closeCommand(3);
		}
		break;
	}
}

//Hang Call main function.		
int SIM808VoiceProvider::hangCall(){
	SIM808ModemCore_t.setCommandError(0);
	SIM808ModemCore_t.setCommandCounter(1);
	SIM808ModemCore_t.openCommand(this, HANGCALL);
	hangCallContinue();
	return SIM808ModemCore_t.getCommandError();
}

//Hang Call continue function.
void SIM808VoiceProvider::hangCallContinue(){
	// 1: ATH
	// 2: Waiting for OK

	bool resp;
	switch (SIM808ModemCore_t.getCommandCounter()) {
	case 1:
		//ATH
		SIM808ModemCore_t.genericCommand_rq(PSTR("ATH"));
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			setvoiceCallStatus(IDLE_CALL);
			if (resp) SIM808ModemCore_t.closeCommand(1);
			else SIM808ModemCore_t.closeCommand(3);
		}
		break;
	}
}

//Response management.
void SIM808VoiceProvider::manageResponse(int from, int to){
	switch (SIM808ModemCore_t.getOngoingCommand()){
	case ANSWERCALL:
		answerCallContinue();
		break;
	case HANGCALL:
		hangCallContinue();
		break;
	case RETRIEVECALLINGNUMBER:
		retrieveCallingNumberContinue();
		break;

	}
}

//URC recognize.
bool SIM808VoiceProvider::recognizeUnsolicitedEvent(int oldTail){
	int nlength;
	char auxLocate[15];
	//RING.
	prepareAuxLocate(PSTR("RING"), auxLocate);
	if (SIM808ModemCore_t.theBuffer().locate(auxLocate)){
		// RING
		setvoiceCallStatus(RECEIVINGCALL);
		SIM808ModemCore_t.theBuffer().Flush();
		return true;
	}

	//CALL ACEPTED.
	prepareAuxLocate(PSTR("+COLP:"), auxLocate);
	if (SIM808ModemCore_t.theBuffer().locate(auxLocate)){
		//DEBUG
#if ANDYDEBUGx
		Serial.println("Call Accepted.");
#endif
		setvoiceCallStatus(TALKING);
		SIM808ModemCore_t.theBuffer().Flush();//
		return true;
	}

	//NO CARRIER.
	prepareAuxLocate(PSTR("NO CARRIER"), auxLocate);
	if (SIM808ModemCore_t.theBuffer().locate(auxLocate)){
		//DEBUG
		//Serial.println("NO CARRIER received.");
		setvoiceCallStatus(IDLE_CALL);
		SIM808ModemCore_t.theBuffer().Flush();
		return true;
	}

	//BUSY.
	prepareAuxLocate(PSTR("BUSY"), auxLocate);
	if (SIM808ModemCore_t.theBuffer().locate(auxLocate)){
		//DEBUG	
		//Serial.println("BUSY received.");
		setvoiceCallStatus(IDLE_CALL);
		SIM808ModemCore_t.theBuffer().Flush();
		return true;
	}

	//CALL RECEPTION.
	prepareAuxLocate(PSTR("+CLIP:"), auxLocate);
	if (SIM808ModemCore_t.theBuffer().locate(auxLocate)){
		SIM808ModemCore_t.theBuffer().Flush();
		setvoiceCallStatus(RECEIVINGCALL);
		return true;
	}

	return false;
}


