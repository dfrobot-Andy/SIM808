/*
	Serial demo
	
	
 created July 2015
 by Andy Zhou
*/

void setup()
{
	Serial.begin(115200); 
	Serial1.begin(115200);
}
void loop()    
{ 
	while(Serial1.available())
	{ 
		Serial.write(Serial1.read());
	}
	delay(1);
	while(Serial.available())
	{ 
		Serial1.write(Serial.read());
	}
	delay(1);
}
