#include <sim808ModemCore.h>
#include <Arduino.h>

SIM808ModemCore SIM808ModemCore_t;

char* __ok__ = "OK";

SIM808ModemCore::SIM808ModemCore() :gss(){
	gss.registerMgr(this);
	_dataInBufferFrom = 0;
	_dataInBufferTo = 0;
	commandError = 1;
	commandCounter = 0;
	ongoingCommand = NONE;
	takeMilliseconds();
	for (int i = 0; i < UMPROVIDERS; i++){
		UMProvider[i] = 0;
	}
}

//
void SIM808ModemCore::registerUMProvider(SIM808BaseProvider* provider){
	for (uint8_t i = 0; i < UMPROVIDERS; i++){
		if (UMProvider[i] == 0){
			UMProvider[i] = provider;
			break;
		}
	}
}

//SIM808BaseProvider
void SIM808ModemCore::unRegisterUMProvider(SIM808BaseProvider* provider){
	for (uint8_t i = 0; i < UMPROVIDERS; i++){
		if (UMProvider[i] == provider){
			UMProvider[i] = 0;
			break;
		}
	}
}

//response parse
bool SIM808ModemCore::genericParse_rsp(bool& rsp, char* str1, char* str2){
	if ((str1 == 0) && (str2 == 0))	str1 = __ok__;

	rsp = theBuffer().locate(str1);
	if ((!rsp) && (str2 != 0))	rsp = theBuffer().locate(str2);

	return true;
}

bool SIM808ModemCore::genericParse_rsp(bool& rsp, char* str1, uint8_t slen){
	if (slen)
		rsp = theBuffer().locate(str1,slen);
	return true;
}

void SIM808ModemCore::closeCommand(int code){
	if ((code != 1) && (SIM808ModemCore_t.getOngoingCommand() == MODEMCONFIG)){
		SIM808ModemCore_t.setStatus(ERROR);
	}
	
	setCommandError(code);
	ongoingCommand = NONE;
	activeProvider = 0;
	commandCounter = 1;
}

void SIM808ModemCore::genericCommand_rq(/*PROGMEM prog_char*/PGM_P str, bool addCR){
	digitalWrite(13,HIGH);
	theBuffer().Flush();
	writePGM(str,addCR);
	digitalWrite(13, LOW);
}

void SIM808ModemCore::genericCommand_rqc(const char* str, bool addCR){
	theBuffer().Flush();

	print(str);
	if (addCR){
		print("\r\n");
	}
}

void SIM808ModemCore::manageMsg(int from, int to){
	manageMsgNow(from,to);
}

void SIM808ModemCore::manageReceivedData(){
	// Just debugging the non debugging
#if MGERECDEBUGx
	Serial.print("ongoingCommand:");
	Serial.print(ongoingCommand);
	Serial.print("   ct:");
	Serial.println(commandCounter);
#endif
}

void SIM808ModemCore::manageMsgNow(int from, int to){
	bool recognized = false;
	
	for (uint8_t i = 0; (i < UMPROVIDERS) && (!recognized); i++){
		if (UMProvider[i]){
			recognized = UMProvider[i]->recognizeUnsolicitedEvent(from);//
		}
	}

	if ((!recognized) && (activeProvider)){
		activeProvider->manageResponse(from, to);//
	}
}

void SIM808ModemCore::openCommand(SIM808BaseProvider* provider, SIM808_CommandType c){
	/*if (activeProvider != 0){
		activeProvider_old = activeProvider;
	}*/
	activeProvider = provider;
	commandError = 0;
	commandCounter = 1;
	ongoingCommand = c;
	_dataInBufferFrom = 0;
	_dataInBufferTo = 0;
}

size_t SIM808ModemCore::writePGM(/*PROGMEM prog_char*/PGM_P str, bool CR){
	int i = 0;
	char c;

	do{
		c = pgm_read_byte_near(str+i);
		if (c != 0){
			write(c);
		}
		i++;
	} while (c != 0);
	if (CR){
		print("\r\n");
	}

	return 1;
}

size_t SIM808ModemCore::write(uint8_t c){
#if ANDYDEBUG
	SIM808CircularBuffer::printCharDebug(c);
#endif
	return gss.write(c);
}
/*
size_t SIM808ModemCore::write(const char* c){
	for (char i = 0; i < strlen(c); ++i){
		write(c[i]);
	}
}*/
/*
size_t SIM808ModemCore::writeStr(const char* str, uint8_t len){
	char c;
	char i = 0;
	do{
		c = str[i++];
		write(c);
	} while (len--);
	return true;
}*/

unsigned long SIM808ModemCore::takeMilliseconds(){
	unsigned long now = millis();
	unsigned long delta;
	delta = now - milliseconds;
	milliseconds = now;
	return delta;
}

void SIM808ModemCore::delayInsideInterrupt(unsigned long milliseconds){
	for (unsigned long k = 0; k < milliseconds; k++){
		SIM808ModemCore_t.gss.tunedDelay(1000);
	}
}









