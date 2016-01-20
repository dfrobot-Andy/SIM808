/*
 GPS DEMO
 
 created July 2015
 by Andy Zhou
 
 This example is in the public domain.
 */

// libraries
#include "sim808.h"

// PIN Number
#define PINNUMBER ""

// initialize the library instance
SIM808 gsmAccess; // include a 'true' parameter for debug enabled
SIM808GPS mygps;
SIM808TCP mytcp;
String remoteNumber = "";  // the number you will call

char tcpip[]="\"TCP\",\"183.230.40.39\",\"876\"";

char datastreams_id[]="GPS Data";
char device_id[]="135506";
char api_key[] ="Qjvc7i1UwteNzL1jOGZ8VsVJ04gA";

char *gpsbuffer;
char gpsdatalen=0;
void setup()
{

  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("GPS debug...");
  
  // connection state
  boolean notConnected = true;
  
  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==SIM808_READY)
      notConnected = false;
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  
  Serial.println("GSM initialized.");
  delay(2000);
}

void loop()
{
  while(mygps.availableGPS()==0){
    Serial.println("NO GPS data...");
    delay(500);
  }
  mygps.readGPS(gpsbuffer);
  gpsdatalen = strlen(gpsbuffer);
  Serial.println(gpsbuffer);
  delay(100);
  if(mytcp.connectTCP(tcpip)){
	Serial.print("    TCP connected:");
	Serial.println(tcpip);
	mytcp.setServerParameters(datastreams_id,device_id,api_key);
	mytcp.sendTCPdata(gpsbuffer);
	mytcp.disConnectTCP();
  }else{
     Serial.println("Not connect tcp service!");
  }
}

