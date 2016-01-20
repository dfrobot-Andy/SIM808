#include <sim808AccessProvider.h>
#include <sim808MobileNetworkProvider.h>
#include <sim808TCPProvider.h>
#include <sim808TCPService.h>
#include <Arduino.h>

SIM808TCPProvider SIM808TCPProvider_t;

SIM808TCPService::SIM808TCPService(){
	this->flags = 0;
	SIM808TCPProvider_t.initialize();
	SIM808TCPProvider_t.setTCPStatus(IDLE_TCP);//Initialize tcp status
}

void SIM808TCPService::restart(){
	boolean notConnected = true;
	SIM808AccessProvider restartdebug;

	while (notConnected){
		if (restartdebug.begin("") == SIM808_READY){
			notConnected = false;
		}else{
#if ANDYDEBUG
			Serial.println("SIM808 not connected");
#endif
			delay(1000);
		}
	}
	SIM808TCPProvider_t.setTCPStatus(IDLE_TCP);//Initialize tcp status after restart
}

bool SIM808TCPService::connectTCP(char *addr){
/*	if (SIM808TCPProvider_t.getTCPStatus() == CONNECTED){
		delay(100);
		return 1;
	}
	*/
	if (SIM808TCPProvider_t.reStr_t==1){
		SIM808TCPProvider_t.reStr_t = 0;
		flags = 0;
		SIM808TCPProvider_t.setTCPStatus(IDLE_TCP);
		this->restart();
		return 0;
	}
	
	if (SIM808TCPProvider_t.getTCPStatus() == LAUNCHED){//
#if ANDYDEBUG
		Serial.println("----->>TCP has launched.");
#endif
		flags++;
		if (flags==1){
			if (SIM808TCPProvider_t.disConnectTCP() == 1){ //
				flags = 0;//
				SIM808TCPProvider_t.setTCPStatus(IDLE_TCP);
#if ANDYDEBUG
				Serial.println("TCP closeed SUCCESS...");
#endif
			}
#if ANDYDEBUG
			else
				Serial.println("TCP closeed bad...");
#endif
		}
		
		if (flags > 1){
			SIM808TCPProvider_t.setTCPStatus(IDLE_TCP);
			SIM808TCPProvider_t.reStr_t = 1;
			delay(100);
		}
		return 0;
	}
	
#if ANDYDEBUG
	Serial.println();
	Serial.println("------------------------------");
	Serial.println("   Start to TCP Connectting");
	Serial.print("       ");
	Serial.println(flags);
	Serial.println("------------------------------");
#endif
	SIM808TCPProvider_t.setTCPStatus(LAUNCHED);//
	return SIM808TCPProvider_t.connectTCP(addr);
}

bool SIM808TCPService::setServerParameters(char* idName, char* idDev, char* pKey){
	if (SIM808TCPProvider_t.getTCPStatus() != CONNECTED){
		return false;
	}
	return SIM808TCPProvider_t.setServerParameters(idName,idDev,pKey);
}

bool SIM808TCPService::disConnectTCP(){
	if (SIM808TCPProvider_t.getTCPStatus() != CONNECTED){
		return false;
	}
	//SIM808TCPProvider_t.delayInternal();
	return SIM808TCPProvider_t.disConnectTCP();
}

bool SIM808TCPService::sendTCPdata(char *dta){
	if (SIM808TCPProvider_t.getTCPStatus() != SETTING){
		return false;
	}
	return SIM808TCPProvider_t.sendTCPdata(dta);
}


