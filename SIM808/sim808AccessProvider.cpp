#include <sim808AccessProvider.h> 
#include <sim808Timer2Serial.h>
#include <Arduino.h>

#define __RESETPIN__ 12
#define __TOUTSHUTDOWN__ 5000
#define __TOUTMODEMCONFIGURATION__ 5000//equivalent to 30000 because of time in interrupt routine.
#define __TOUTAT__ 1000

const char _command_AT[] PROGMEM = "AT";
const char _command_CGREG[] PROGMEM = "AT+CGREG?";

SIM808AccessProvider::SIM808AccessProvider(bool debug){
	SIM808ModemCore_t.setDebug(debug);
	flags_t = false;
}
/*
SIM808AccessProvider::~SIM808AccessProvider(){
	if (SIM808ModemCore_t.activeProvider_old != 0){
		SIM808ModemCore_t.activeProvider = SIM808ModemCore_t.activeProvider_old;
	}
}
*/
void SIM808AccessProvider::manageResponse(int from, int to){
	switch (SIM808ModemCore_t.getOngoingCommand())
	{
	case MODEMCONFIG:
#if SERIALREC
		Serial.println("received data...");
#else
		ModemConfigurationContinue();
#endif
		break;
	case ALIVETEST:
		isModemAliveContinue();
		break;
	default:
		break;
	}
}

char flags_st = 0;
SIM800_NetworkStatus_t SIM808AccessProvider::begin(char* pin, bool restart, bool synchronous){
	pinMode(__RESETPIN__,OUTPUT);
	pinMode(13, OUTPUT);

	if (restart){
#if ANDYDEBUG
		Serial.println("restart...");//debug print
#endif
		HWrestart();
	}else{
		HWstart();
	}
	delay(200);
	//start timer and Serial1 int...
	SIM808ModemCore_t.gss.begin(19200);//100ms
	
	ModemConfiguration(pin);
	flags_st = 0;
	if (synchronous){
		while (ready() == 0){
			delay(1500);
			if (flags_st > 20){
				SIM808ModemCore_t.closeCommand(3);
				break;
			}
			flags_st++;
		}
	}
	return getStatus();
}

int SIM808AccessProvider::HWrestart(){
	SIM808ModemCore_t.setStatus(IDLE);

	digitalWrite(__RESETPIN__,HIGH);
	digitalWrite(13, HIGH);
	delay(2000);
	digitalWrite(__RESETPIN__,LOW);
	digitalWrite(13, LOW);
	delay(1000);
	return 1;
}

int SIM808AccessProvider::HWstart(){
	SIM808ModemCore_t.setStatus(IDLE);

	digitalWrite(__RESETPIN__, HIGH);
	delay(2000);
	digitalWrite(__RESETPIN__, LOW);
	return 1;
}

int SIM808AccessProvider::ModemConfiguration(char* pin){
	SIM808ModemCore_t.setPhoneNumber(pin);
	SIM808ModemCore_t.openCommand(this,MODEMCONFIG);
	SIM808ModemCore_t.setStatus(CONNECTING);
	ModemConfigurationContinue();

	return SIM808ModemCore_t.getCommandError();
}

void SIM808AccessProvider::ModemConfigurationContinue(){
	bool resp;
	//ct--->>>commandCounter
	int ct = SIM808ModemCore_t.getCommandCounter();
	flags_st = 0;
	if (ct == 1){		
		//Æô¶¯AT
		char i = 0;
		SIM808ModemCore_t.setCommandCounter(2);
		while (1){
			SIM808ModemCore_t.genericCommand_rq(_command_AT);//			
			i++;
			delay(1000);
			if (flags_t){
				break;
			}
			if (i > 1){
				SIM808ModemCore_t.closeCommand(3);
				break;
			}
		}
	}else if (ct == 2){
		if (SIM808ModemCore_t.genericParse_rsp(resp))//
		{
			if (resp)
			{		// OK received
				if (SIM808ModemCore_t.getPhoneNumber() && (SIM808ModemCore_t.getPhoneNumber()[0] != 0))//
				{
					SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CPIN="),false);
					SIM808ModemCore_t.setCommandCounter(3);
					SIM808ModemCore_t.genericCommand_rqc(SIM808ModemCore_t.getPhoneNumber());
				}else{//received "OK"
					SIM808ModemCore_t.setCommandCounter(11);
					SIM808ModemCore_t.takeMilliseconds();
					SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CIURC=1"));//¡°Call Ready¡±
				}
			}else {
				SIM808ModemCore_t.closeCommand(3);
			}
		}
	}else if (ct == 3){
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			if (resp){
				SIM808ModemCore_t.setCommandCounter(4);
				SIM808ModemCore_t.takeMilliseconds();
				SIM808ModemCore_t.delayInsideInterrupt(2000);
				SIM808ModemCore_t.genericCommand_rq(_command_CGREG);
			}else{
				SIM808ModemCore_t.closeCommand(3);
			}
		}
	}else if (ct == 4){
		char auxLocate1[12];
		char auxLocate2[12];
		prepareAuxLocate(PSTR("+CGREG: 0,0"), auxLocate1);//
		prepareAuxLocate(PSTR("+CGREG: 0,5"), auxLocate2);//
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate1, auxLocate2)){
			if (resp){//
				SIM808ModemCore_t.setCommandCounter(5);
				SIM808ModemCore_t.genericCommand_rq(PSTR("AT+IFC=1,1"));//
			}else{
				if (SIM808ModemCore_t.takeMilliseconds() > __TOUTMODEMCONFIGURATION__){//
					SIM808ModemCore_t.closeCommand(3);
				}else{
					SIM808ModemCore_t.delayInsideInterrupt(2000);
					SIM808ModemCore_t.genericCommand_rq(PSTR("AT+IFC=1,1"));//
				}
			}
		}
	}else if (ct == 5){
		// 5: Wait IFC OK
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			//Delay for SW flow control being active.
			SIM808ModemCore_t.delayInsideInterrupt(2000);
			// 9: SMS Text Mode
			SIM808ModemCore_t.setCommandCounter(6);
			SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CMGF=1"));
		}
	}else if (ct == 6){
		// 6: Wait SMS text Mode OK
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			//Calling line identification
			if (resp){
				SIM808ModemCore_t.setCommandCounter(7);
				SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CLIP=1"));
			}else{
				SIM808ModemCore_t.delayInsideInterrupt(2000);
				SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CMGF=1"));//
			}	
		}
	}else if (ct == 7){
		// 7: Wait Calling Line Id OK
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			SIM808ModemCore_t.setCommandCounter(8);
			SIM808ModemCore_t.genericCommand_rq(PSTR("ATE0"));//
		}
	}else if (ct == 8){
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			SIM808ModemCore_t.setCommandCounter(9);
			SIM808ModemCore_t.genericCommand_rq(PSTR("AT+COLP=1"));
		}
	}else if (ct == 9){
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			SIM808ModemCore_t.setCommandCounter(10);
			SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CSQ"));
		}	
	}else if (ct == 10){
		char auxLocate[12];
		prepareAuxLocate(PSTR("+CSQ: 0,0"), auxLocate);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate)){
			if (resp){
				SIM808ModemCore_t.delayInsideInterrupt(2000);
				SIM808ModemCore_t.setCommandCounter(10);
				SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CSQ"));	//	
			}else{
				flags_t = true;
				SIM808ModemCore_t.setCommandCounter(12);
			}
		}
	}else if (ct == 11){
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			SIM808ModemCore_t.setCommandCounter(4);
			SIM808ModemCore_t.genericCommand_rq(_command_CGREG);
		}
	}
	else if (ct == 12){
		char auxLocate[12];
		prepareAuxLocate(PSTR("Call Ready"), auxLocate);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate)){
			if (resp){
#if ANDYDEBUG
				Serial.println("-------->>>>>>>>>>CALL READY...");
				Serial.println("-------->>>>>>>>Kernel Initialization is Complete...");
#endif
				SIM808ModemCore_t.setCommandCounter(13);
				//SIM808ModemCore_t.setStatus(SIM808_READY);
				//SIM808ModemCore_t.closeCommand(1);
			}else{			
				SIM808ModemCore_t.closeCommand(3);
			}
		}
	}
	else if (ct == 13){
		char auxLocate[12];
		prepareAuxLocate(PSTR("Call Ready"), auxLocate);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate)){
			if (resp){
#if ANDYDEBUG
				Serial.println("-------->>>>>>>>>>SMS READY...");
				Serial.println("-------->>>>>>>>Kernel Initialization is Complete...");
#endif
				SIM808ModemCore_t.setStatus(SIM808_READY);
				SIM808ModemCore_t.closeCommand(1);
			}
			else{
				SIM808ModemCore_t.closeCommand(3);
			}
		}
	}
}

int SIM808AccessProvider::isAccessAlive(){
	SIM808ModemCore_t.setCommandError(0);
	SIM808ModemCore_t.setCommandCounter(1);
	SIM808ModemCore_t.openCommand(this,ALIVETEST);
	isModemAliveContinue();
	
	return SIM808ModemCore_t.getCommandError();
}

void SIM808AccessProvider::isModemAliveContinue(){
	bool rsp;
	switch (SIM808ModemCore_t.getCommandCounter()){
	case 1:
		SIM808ModemCore_t.genericCommand_rq(_command_AT);
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		if (SIM808ModemCore_t.genericParse_rsp(rsp)){
			SIM808ModemCore_t.setCommandCounter(4);
			SIM808ModemCore_t.takeMilliseconds();
			SIM808ModemCore_t.genericCommand_rq(_command_CGREG);
		}
		break;
	}
}

bool SIM808AccessProvider::shutdown()
{
	unsigned long m;
	bool resp;
	char auxLocate[12];

	// It makes no sense to have an asynchronous shutdown
	pinMode(__RESETPIN__, OUTPUT);
	digitalWrite(__RESETPIN__, HIGH);
	delay(1500);
	digitalWrite(__RESETPIN__, LOW);
	SIM808ModemCore_t.setStatus(IDLE);
	SIM808ModemCore_t.gss.close();

	m = millis();
	prepareAuxLocate(PSTR("POWER DOWN"), auxLocate);
	while ((millis() - m) < __TOUTSHUTDOWN__){
		delay(1);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate))
			return resp;
	}
	return false;
}











