#include <sim808SMSProvider.h>
#include <Arduino.h>  

SIM808SMSProvider::SIM808SMSProvider(){
	SIM808MobileSMSProvider_p = this;
}

int SIM808SMSProvider::beginSMS(const char* to){
	if ((SIM808ModemCore_t.getStatus() != SIM808_READY) && (SIM808ModemCore_t.getStatus() != GPRS_READY)){
		return 2;
	}
	SIM808ModemCore_t.setPhoneNumber((char*)to);
	SIM808ModemCore_t.openCommand(this,BEGINSMS);
	beginSMSContinue();

	return SIM808ModemCore_t.getCommandError();
}

void SIM808SMSProvider::beginSMSContinue(){
	bool resp;
	switch (SIM808ModemCore_t.getCommandCounter()){
	case 1://
		SIM808ModemCore_t.setCommandCounter(2);
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CMGS=\""),false);
		SIM808ModemCore_t.print(SIM808ModemCore_t.getPhoneNumber());
		SIM808ModemCore_t.print("\"\r");
		break;
	case 2://
		if (SIM808ModemCore_t.genericParse_rsp(resp, ">")){
			if (resp){
				SIM808ModemCore_t.closeCommand(1);
			}else{
				SIM808ModemCore_t.closeCommand(3);
			}
			break;
		}
	}
}

void SIM808SMSProvider::writeSMS(char c){
	SIM808ModemCore_t.write(c);
}

int SIM808SMSProvider::endSMS(){
	SIM808ModemCore_t.openCommand(this,ENDSMS);
	endSMSContinue();
	return SIM808ModemCore_t.getCommandError();
}

void SIM808SMSProvider::endSMSContinue()
{
	bool resp;
	// 1: Send #26
	// 2: wait for OK
	switch (SIM808ModemCore_t.getCommandCounter()) {
	case 1:
		SIM808ModemCore_t.setCommandCounter(2);
		SIM808ModemCore_t.write(26);
		SIM808ModemCore_t.print("\r");
		break;
	case 2:
		if (SIM808ModemCore_t.genericParse_rsp(resp))
		{
			if (resp){
				SIM808ModemCore_t.closeCommand(1);
			}else{
				SIM808ModemCore_t.closeCommand(3);
			}
		}
		break;
	}
}

//Available SMS main function.
int SIM808SMSProvider::availableSMS()
{
	flagReadingSMS = 0;
	SIM808ModemCore_t.openCommand(this, AVAILABLESMS);
	availableSMSContinue();
	return SIM808ModemCore_t.getCommandError();
}

//Available SMS continue function.
void SIM808SMSProvider::availableSMSContinue()
{
	// 1:  AT+CMGL="REC UNREAD",1
	// 2: Receive +CMGL: _id_ ... READ","_numero_" ... \n_mensaje_\nOK
	// 3: Send AT+CMGD= _id_
	// 4: Receive OK
	// 5: Remaining SMS text in case full buffer.
	// This implementation really does not care much if the modem aswers trash to CMGL
	bool resp;
	//int msglength_aux;
	switch (SIM808ModemCore_t.getCommandCounter()) {
	case 1:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CMGL=\"REC UNREAD\",1"));
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		if (parseCMGLAvailable(resp))
		{
			if (!resp){
				SIM808ModemCore_t.closeCommand(4);
			}else{
				SIM808ModemCore_t.closeCommand(1);
			}
		}
		break;
	}

}

//SMS available parse.
bool SIM808SMSProvider::parseCMGLAvailable(bool& rsp)
{
	fullBufferSMS = (SIM808ModemCore_t.theBuffer().availableBytes() <= 4);
	if (!(SIM808ModemCore_t.theBuffer().chopUntil("+CMGL:", true))){
		rsp = false;
	}else{
		rsp = true;
	}
	idSMS = SIM808ModemCore_t.theBuffer().readInt();

	//If there are 2 SMS in buffer, response is ...CRLFCRLF+CMGL
	twoSMSinBuffer = SIM808ModemCore_t.theBuffer().locate("\r\n\r\n+");

	checkSecondBuffer = 0;

	return true;
}

//remoteNumber SMS function.
int SIM808SMSProvider::remoteSMSNumber(char* number, int nlength)
{
	SIM808ModemCore_t.theBuffer().extractSubString("READ\",\"", "\"", number, nlength);

	return 1;
}

//remoteNumber SMS function.
int SIM808SMSProvider::readSMS()
{
	char charSMS;
	//First char.
	if (!flagReadingSMS)
	{
		flagReadingSMS = 1;
		SIM808ModemCore_t.theBuffer().chopUntil("\n", true);
	}
	charSMS = SIM808ModemCore_t.theBuffer().Read();

	//Second Buffer.
	if (checkSecondBuffer)
	{
		checkSecondBuffer = 0;
		twoSMSinBuffer = SIM808ModemCore_t.theBuffer().locate("\r\n\r\n+");
	}

	//Case the last char in buffer.
	if ((!twoSMSinBuffer) && fullBufferSMS && (SIM808ModemCore_t.theBuffer().availableBytes() == 127))
	{
		SIM808ModemCore_t.theBuffer().Flush();
		fullBufferSMS = 0;
		checkSecondBuffer = 1;
		SIM808ModemCore_t.openCommand(this, XON);
		SIM808ModemCore_t.gss.spaceAvailable();
		delay(10);

		return charSMS;
	}
	//Case two SMS in buffer
	else if (twoSMSinBuffer)
	{
		if (SIM808ModemCore_t.theBuffer().locate("\r\n\r\n+")){
			return charSMS;
		}else{
			SIM808ModemCore_t.theBuffer().Flush();
			SIM808ModemCore_t.openCommand(this, XON);
			SIM808ModemCore_t.gss.spaceAvailable();
			delay(10);
			return 0;
		}
	}
	//Case 1 SMS and buffer not full
	else if (!fullBufferSMS){
		if (SIM808ModemCore_t.theBuffer().locate("\r\n\r\nOK")){
			return charSMS;
		}else{
			SIM808ModemCore_t.theBuffer().Flush();
			SIM808ModemCore_t.openCommand(this, XON);
			SIM808ModemCore_t.gss.spaceAvailable();
			delay(10);
			return 0;
		}
	}
	//Case to read all the chars in buffer to the end.
	else{
		return charSMS;
	}
}

//Read socket main function.
int SIM808SMSProvider::peekSMS()
{
	if (!flagReadingSMS)
	{
		flagReadingSMS = 1;
		SIM808ModemCore_t.theBuffer().chopUntil("\n", true);
	}

	return SIM808ModemCore_t.theBuffer().Peek(0);
}

//Flush SMS main function.
void SIM808SMSProvider::flushSMS()
{

	//With this, sms data can fill up to 2x128+5x128 bytes.
	for (int aux = 0; aux<5; aux++)
	{
		SIM808ModemCore_t.theBuffer().Flush();
		SIM808ModemCore_t.gss.spaceAvailable();
		delay(10);
	}

	SIM808ModemCore_t.openCommand(this, FLUSHSMS);
	flushSMSContinue();
}

//Send SMS continue function.
void SIM808SMSProvider::flushSMSContinue()
{
	bool resp;
	// 1: Deleting SMS
	// 2: wait for OK
	switch (SIM808ModemCore_t.getCommandCounter()) {
	case 1:
		SIM808ModemCore_t.setCommandCounter(2);
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CMGD="), false);
		SIM808ModemCore_t.print(idSMS);
		SIM808ModemCore_t.print("\r");
		break;
	case 2:
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			if (resp) SIM808ModemCore_t.closeCommand(1);
			else SIM808ModemCore_t.closeCommand(3);
		}
		break;
	}
}

void SIM808SMSProvider::manageResponse(int from, int to)
{
	switch (SIM808ModemCore_t.getOngoingCommand()){	
	    case NONE:
			SIM808ModemCore_t.gss.cb.deleteToTheEnd(from);
			break;
		case BEGINSMS:
			beginSMSContinue();
			break;
		case ENDSMS:
			endSMSContinue();
			break;
		case AVAILABLESMS:
			availableSMSContinue();
			break;
		case FLUSHSMS:
			flushSMSContinue();
			break;
	}
#if ANDYDEBUGx
	Serial.println("SMS getOngoingCommand...");
	Serial.println(SIM808ModemCore_t.getOngoingCommand());
#endif
}


