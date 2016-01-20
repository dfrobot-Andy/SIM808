#include <sim808GPSProvider.h>
#include <Arduino.h>


SIM808GPSProvider::SIM808GPSProvider(){
	SIM808MobileGPSProvider_p = this;
	this->gpsStatus_st = IDLE_GPS;
}

void SIM808GPSProvider::beginModemConfigurationCB(){
	bool resp;
	//ct--->>>commandCounter
	switch (SIM808ModemCore_t.getCommandCounter()){
	case 1:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CGNSPWR=1"));//
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		if (SIM808ModemCore_t.genericParse_rsp(resp)){
			if (resp){
				SIM808ModemCore_t.setCommandCounter(3);
				SIM808ModemCore_t.takeMilliseconds();
				SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CGNSTST=1"));
			}
		}
		break;
	case 3:
		if (SIM808ModemCore_t.genericParse_rsp(resp)){        //
			if (resp){		
#if 1
				SIM808ModemCore_t.closeCommand(1);
			//	SIM808ModemCore_t.setCommandError(1);
#endif
			}else{
				SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CGNSTST=1"));//
			}
		}
		break;
	}	
}

void SIM808GPSProvider::endModemConfigurationCB(){
	bool resp;
	//ct--->>>commandCounter
	switch (SIM808ModemCore_t.getCommandCounter()){
	case 1:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CGNSTST=0"));//
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	case 2:
		if (SIM808ModemCore_t.genericParse_rsp(resp)){//OK
			if (resp){			
				Serial.println("close gps display ... ... ...");
				//SIM808ModemCore_t.setCommandError(1);
				SIM808ModemCore_t.closeCommand(1);
			}
		}
		break;
	case 3:
		SIM808ModemCore_t.genericCommand_rq(PSTR("AT+CGNSPWR=0"));//
		SIM808ModemCore_t.setCommandCounter(2);
		break;
	}
}

void SIM808GPSProvider::manageResponse(int from, int to){
	switch (SIM808ModemCore_t.getOngoingCommand())
	{
	case NONE:
		SIM808ModemCore_t.gss.cb.deleteToTheEnd(from);
		break;
	case BEGINGPS:
		beginModemConfigurationCB();
		break;
	case ENDGPS:
		endModemConfigurationCB();
		break;
	case BEGINGPRS:

		break;
	case ENDGPRS:

		break;
	case CHECKPOWER:

		break;
	default:
		break;
	}
}

void SIM808GPSProvider::initialize(){
	SIM808ModemCore_t.registerUMProvider(this);//
}

bool SIM808GPSProvider::locateString(const char* reference, const char* original, char* from){
	char b2 = 0;
	char index_p = 0;
	char index_t = 0;
	if (reference[0] == 0){
		return true;
	}
	for (char b1 = 0; b1 != strlen(original); ++b1){
		if (original[b1] == reference[b2]){
			if (original[b1 + 1] == reference[b2 + 1]){
				b1 += 2;
				//memcpy(from,"debug",6);
				while (original[b1] != ','){
					*from = original[b1];
					if (*from == '.'){
						index_p = index_t;
					}
					index_t++;
					from++;
					b1++;
				}
				from -= index_t;
				from[index_p] = from[index_p - 1];
				from[index_p-1] = from[index_p - 2];
				from[index_p-2] = '.';
				return true;
			}
		}
	}
	return false;
}

bool SIM808GPSProvider::locateString(const char* reference, const char* original){
	char b2 = 0;
	if (reference[0] == 0){
		return true;
	}
	for (char b1 = 0; b1 != strlen(original); ++b1){
		if (original[b1] == reference[b2]){
			b2++;
			if (reference[b2] == 0){
				return true;
			}
		}
	}
	return false;
}

/*
"\
$GPGGA,001118.261,xxx,xxx,xxx,xxx,0,0,xxx,xxx,M,xxx,M,xxx,*44\r\n\
$GPGLL,,xx,xxx,xxx,001118.261,V,N*76\r\n\
$GPGSA,A,1,xxxx,,xx,,xx,,,xx,,,xx,,xx,,*1E\r\n\
$GPGSV,1,1,00*79\r\n\
$GPRMC,021800.000,A,3040.0607,N,10548.5315,E,0.00,0.00,060180,,,N*4E\r\n\
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32\r\n\
"
*/
bool SIM808GPSProvider::retrieveGPSdta(char* buffer, int bufsize){
#if GPSDEBUGx
	strcpy(buffer, "$GPRMC,021800.000,A,3040.0607,N,10548.5315,E,0.00,0.00,060180,,,N*4E");
					
#else
	SIM808ModemCore_t.theBuffer().extractSubString("GPRMC,", "\r\n", buffer, bufsize);
#endif
	SIM808ModemCore_t.theBuffer().Flush();
}

bool SIM808GPSProvider::recognizeUnsolicitedEvent(int oldTail){//
	//char gprmc[100] = { 0 };
	char auxLocate[15];
	
	prepareAuxLocate(PSTR("$GPRMC,"), auxLocate);
	if (SIM808ModemCore_t.theBuffer().locate(auxLocate)){
		this->retrieveGPSdta(this->gpsRep,100);
#if ANDYDEBUG
		Serial.println(this->gpsRep); //
#endif
		if (this->locateString("A,", this->gpsRep)){
			this->setGPSStatus(GPSREV);
			SIM808ModemCore_t.setStatus(GPS_READY);
		}
		SIM808ModemCore_t.setCommandError(1);
		SIM808ModemCore_t.theBuffer().Flush();
		return true;
	}
	this->setGPSStatus(NODATA);
	return false;
}

bool SIM808GPSProvider::availableGPS(){
	return ((this->getGPSStatus() == GPSREV) ? true : false);
}

bool SIM808GPSProvider::getGPSdata(char *str){
	if (str == 0){
		return false;
	}
	if (this->getGPSStatus() == GPSREV){
		str = this->gpsRep;//
		return true;
	}
	return false;
}

void SIM808GPSProvider::start(){
	SIM808ModemCore_t.openCommand(this,BEGINGPS);
	SIM808ModemCore_t.setStatus(CONNECTING);
	SIM808ModemCore_t.setCommandCounter(1);
	this->setGPSStatus(NODATA);
	beginModemConfigurationCB();
}

void SIM808GPSProvider::stop(){
	SIM808ModemCore_t.setOngoingCommand(ENDGPS);
	SIM808ModemCore_t.setCommandCounter(1);
	endModemConfigurationCB();
}

void SIM808GPSProvider::stopGPSPWD(){
	SIM808ModemCore_t.openCommand(this, ENDGPS);
	SIM808ModemCore_t.setCommandCounter(3);
	endModemConfigurationCB();
}


