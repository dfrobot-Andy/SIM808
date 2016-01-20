#include <sim808BaseProvider.h>
//#include <sim808ModemCore.h>
//#include <Arduino.h>

void SIM808BaseProvider::prepareAuxLocate(/*PROGMEM prog_char*/PGM_P str, char auxLocate[]){
	int i = 0;
	char c;

	do
	{
		c = pgm_read_byte_near(str + i);
		auxLocate[i] = c;
		i++;
	} while (c != 0);
}
