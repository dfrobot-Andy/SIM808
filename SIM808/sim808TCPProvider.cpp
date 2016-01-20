#include <sim808TCPProvider.h>
#include <Arduino.h>

#define __TOUT__	15000

#if 0
char _TCP_GPS[] PROGMEM = "POST /v1.0/device/22911/sensor/39908/datapoints HTTP/1.1\r\n\
Host: api.yeelink.net\r\n\
Accept: */*\r\n\
U-ApiKey: c3434e19530ce4090b29d62b9cb1d3aa\r\n\
Content-Length: ";//
char _TCP_DATA[] PROGMEM = "Connection: close\r\n\r\n";//
#endif

const char _streamA_[] PROGMEM = "{\"datastreams\":[{\"id\":\"";
const char _streamB_[] PROGMEM = "\",\"datapoints\":[{\"value\":\"";
const char _streamC_[] PROGMEM = "\"}]}]}";

SIM808TCPProvider::SIM808TCPProvider(){
	this->pdta = 0;
	this->reStr_t = 0;
	this->gpsRep = 0;
	this->n = millis();
	SIM808MobileTCPProvider_p = this;
}

void SIM808TCPProvider::manageResponse(int from, int to){
	switch (SIM808ModemCore_t.getOngoingCommand()){
	case CONNECTTCPCLIENT:
		this->TCPModemConfigurationCB();
		break;
	case TCPTRANSPORT:
		this->TCPTransmissionCB();
		break;
	default:
		break;
	}
}

void SIM808TCPProvider::initialize(){
	SIM808ModemCore_t.registerUMProvider(this);
}



//"AT+CIPSTART=\"TCP\",\"42.96.164.52\",\"80\""
//if connected return true
//CLOSED
bool SIM808TCPProvider::connectTCP(char *addr){
	this->tcpip = addr;
	SIM808ModemCore_t.openCommand(this, CONNECTTCPCLIENT);
	SIM808ModemCore_t.setStatus(CONNECTING);//
	TCPModemConfigurationCB();
	
	return this->waitForAnswer(1);//
}

bool SIM808TCPProvider::disConnectTCP(){
	SIM808ModemCore_t.openCommand(this, CONNECTTCPCLIENT);
	SIM808ModemCore_t.setStatus(CONNECTING);//
	SIM808ModemCore_t.setCommandCounter(4);
	TCPModemConfigurationCB();
	return this->waitForAnswer(1);
}

//
bool SIM808TCPProvider::sendTCPdata(char *dta){
	if (this->getTCPStatus() != SETTING){
		return false;
	}
	this->setTCPStatus(CONNECTED);
	SIM808ModemCore_t.openCommand(this, TCPTRANSPORT);
	SIM808ModemCore_t.setStatus(CONNECTING);//
	this->pdta = dta;
	TCPTransmissionCB();

	return this->waitForAnswer(1);//
}

bool SIM808TCPProvider::setServerParameters(char* idName, char* idDev, char* pKey){
	if (this->getTCPStatus() != CONNECTED){
		return false;
	}
	this->idNames = idName;
	this->idDevice = idDev;
	this->pKeys = pKey;
	this->tcpLen = strlen(pKey) + strlen(idDev)+13;
	this->setTCPStatus(SETTING);
	SIM808ModemCore_t.openCommand(this, TCPTRANSPORT);
	SIM808ModemCore_t.setStatus(CONNECTING);//
	
	TCPTransmissionCB();

	return this->waitForAnswer(1);//
}

bool SIM808TCPProvider::waitForAnswer(int reValue){
	unsigned long m;
	m = millis();
	while (((millis() - m) < __TOUT__) && (ready() == 0)){//ready
		delay(100);
	}
	if (ready() == 1){
		return true;
	}else{
		return false;
	}
}

unsigned long SIM808TCPProvider::TCPSendTime(void){
	unsigned long m;
	m = millis()-n;
	n = millis();
	return m;
}

void SIM808TCPProvider::delayInternal(){
	SIM808ModemCore_t.delayInsideInterrupt(1000);
}

unsigned long nums = 0;
bool SIM808TCPProvider::recognizeUnsolicitedEvent(int oldTail){
	int nlength;
	char auxLocate[20];
	//RING.
	prepareAuxLocate(PSTR("CLOSED"), auxLocate);
	if (SIM808ModemCore_t.theBuffer().locate(auxLocate)){
		SIM808ModemCore_t.setStatus(TCP_DISCONNECTED);//
		this->setTCPStatus(DISCONNECTED);

		SIM808ModemCore_t.closeCommand(1);
		SIM808ModemCore_t.theBuffer().Flush();
		return true;
	}

	return false;
}

void SIM808TCPProvider::TCPTransmissionCB(){
	bool resp;
	char auxLocate[10];
	switch (SIM808ModemCore_t.getCommandCounter()){
	case 1:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CIPSEND"));//
		SIM808ModemCore_t.setCommandCounter(2);//send data
		break;
	case 2:
		prepareAuxLocate(PSTR(">"), auxLocate);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate)){
			if (resp){
				if (this->tcpStatus_t == SETTING){
					uint8_t dta[10] = { 0x00, 0x03, 0x45, 0x44, 0x50, 0x01, 0x40, 0x01, 0x00, 0x00 };
					SIM808ModemCore_t.write(0x10);//
					SIM808ModemCore_t.write(this->tcpLen);//--------2
					for (char i = 0; i < 10; ++i){//--------3
						SIM808ModemCore_t.write(dta[i]);
					}
					SIM808ModemCore_t.write(strlen(this->idDevice));//
					SIM808ModemCore_t.genericCommand_rqc(this->idDevice, false);//--------5  135506
					SIM808ModemCore_t.write(0x00);
					SIM808ModemCore_t.write(strlen(this->pKeys));
					SIM808ModemCore_t.genericCommand_rqc(this->pKeys, false);
				}else{//
					uint8_t strLens = strlen(this->pdta)+strlen(this->idNames)+59;
					SIM808ModemCore_t.write(0x80);//--------1
					SIM808ModemCore_t.write(strLens);
					if (strLens>127){
						SIM808ModemCore_t.write(0x01);
					}
					SIM808ModemCore_t.write(0x00);
					SIM808ModemCore_t.write(0x01);//type
					SIM808ModemCore_t.write(0x00);
					SIM808ModemCore_t.write(strLens-4);
					SIM808ModemCore_t.genericCommand_rq(_streamA_, false);////"{\"datastreams\":[{\"id\":\""
					SIM808ModemCore_t.genericCommand_rqc(this->idDevice, false);//id
					SIM808ModemCore_t.genericCommand_rq(_streamB_, false);//"\",\"datapoints\":[{\"value\":\""
					SIM808ModemCore_t.genericCommand_rqc(this->pdta, false);//GPS data
					SIM808ModemCore_t.genericCommand_rq(_streamC_, false);//

				}
				SIM808ModemCore_t.write(0x1a);
				SIM808ModemCore_t.write(0x0d);
				SIM808ModemCore_t.write(0x0a);
				

				SIM808ModemCore_t.setCommandCounter(3);
			}
		}
		break;
	case 3:
		prepareAuxLocate(PSTR("SEND OK"), auxLocate);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate)){
			if (resp){
				if (this->tcpStatus_t == SETTING){
					memset(auxLocate, 10, 4 * sizeof(char));
					auxLocate[0] = 0x20;
					auxLocate[1] = 0x02;
					if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate,4))
					{
						if (resp)
						{
							SIM808ModemCore_t.closeCommand(1);
						}else{
							SIM808ModemCore_t.setStatus(TCP_CONNECTED);//
							SIM808ModemCore_t.closeCommand(3);
						}				
					}
				}else{
					SIM808ModemCore_t.setStatus(TCP_CONNECTED);
					this->gpsRep = false;
					SIM808ModemCore_t.closeCommand(1);//
				}
			}
		}
		break;
	
	default:
		break;
	}
}

void SIM808TCPProvider::TCPModemConfigurationCB(){
	bool resp;
	char auxLocate[15];
	switch (SIM808ModemCore_t.getCommandCounter()){
	case 1:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CIPSTART="), false);//tcp
		SIM808ModemCore_t.genericCommand_rqc(this->tcpip);
		this->setTCPStatus(LAUNCHED);
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		prepareAuxLocate(PSTR("CONNECT OK"), auxLocate);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate)){
			if (resp){
				SIM808ModemCore_t.setStatus(TCP_CONNECTED);//
				this->setTCPStatus(CONNECTED);
#if ANDYDEBUG
				Serial.println("------------------------------");
				Serial.print("    TCP Connected Success:");
				Serial.print(++nums);
				Serial.print("   ");
				Serial.println(this->TCPSendTime());
				Serial.println("------------------------------");
#endif
				SIM808ModemCore_t.closeCommand(1);
			}
		}
		break;
	case 4:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CIPCLOSE=0"));//tcp
		SIM808ModemCore_t.setCommandCounter(5);
		break;
	case 5:
		prepareAuxLocate(PSTR("CLOSE OK"), auxLocate);
		if (SIM808ModemCore_t.genericParse_rsp(resp, auxLocate)){
			if (resp){
				SIM808ModemCore_t.setStatus(TCP_DISCONNECTED);//
				this->setTCPStatus(DISCONNECTED);
				SIM808ModemCore_t.closeCommand(1);
#if ANDYDEBUG
				Serial.println("TCP closed connected...");
#endif
			}
		}
		break;
	default:
		//
		//this->reStr_t = 1;
		break;
	}
}
