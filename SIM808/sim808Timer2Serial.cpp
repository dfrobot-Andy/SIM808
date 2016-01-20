#include <sim808Timer2Serial.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include <stdio.h>
//#include <inttypeds.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <pins_arduino.h>


#define _SIM808SOFTSERIALFLAGS_ESCAPED_ 0x01
#define _SIM808SOFTSERIALFLAGS_SENTXOFF_ 0x02

#define __PARAGRAPHGUARD__ 50
#if defined(__AVR_ATmega32U4__)
#define __TXPIN__ 1
#define __RXPIN__ 0
#define __RXINT__ 2
#endif

#define __XON__		0x11
#define __XOFF__	0x13

#if 0
#if defined(__AVR_ATmega32U4__)
	#define TIMER_INTR_NAME      TIMER1_OVF_vect
#else	
	#define TIMER_INTR_NAME      TIMER2_OVF_vect
#endif
#endif

typedef struct _DELAY_TABLE
{
	long baud;
	unsigned short rx_delay_centering;
	unsigned short rx_delay_intrabit;
	unsigned short rx_delay_stopbit;
	unsigned short tx_delay;
} DELAY_TABLE;

#if F_CPU == 16000000

static const DELAY_TABLE PROGMEM table[] =
{
	//  baud    rxcenter   rxintra    rxstop    tx
	{ 115200, 1, 17, 17, 12, },
	{ 57600, 10, 37, 37, 33, },
	{ 38400, 25, 57, 57, 54, },
	{ 31250, 31, 70, 70, 68, },
	{ 28800, 34, 77, 77, 74, },
	{ 19200, 54, 117, 117, 114, },
	{ 14400, 74, 156, 156, 153, },
	{ 9600, 114, 236, 236, 233, },
	{ 4800, 233, 474, 474, 471, },
	{ 2400, 471, 950, 950, 947, },
	{ 1200, 947, 1902, 1902, 1899, },
	{ 300, 3804, 7617, 7617, 7614, },
};

const int XMIT_START_ADJUSTMENT = 5;

#elif F_CPU == 8000000

static const DELAY_TABLE table[] PROGMEM =
{
	//  baud    rxcenter    rxintra    rxstop  tx
	{ 115200, 1, 5, 5, 3, },
	{ 57600, 1, 15, 15, 13, },
	{ 38400, 2, 25, 26, 23, },
	{ 31250, 7, 32, 33, 29, },
	{ 28800, 11, 35, 35, 32, },
	{ 19200, 20, 55, 55, 52, },
	{ 14400, 30, 75, 75, 72, },
	{ 9600, 50, 114, 114, 112, },
	{ 4800, 110, 233, 233, 230, },
	{ 2400, 229, 472, 472, 469, },
	{ 1200, 467, 948, 948, 945, },
	{ 300, 1895, 3805, 3805, 3802, },
};

const int XMIT_START_ADJUSTMENT = 4;

#elif F_CPU == 20000000

// 20MHz support courtesy of the good people at macegr.com.
// Thanks, Garrett!

static const DELAY_TABLE PROGMEM table[] =
{
	//  baud    rxcenter    rxintra    rxstop  tx
	{ 115200, 3, 21, 21, 18, },
	{ 57600, 20, 43, 43, 41, },
	{ 38400, 37, 73, 73, 70, },
	{ 31250, 45, 89, 89, 88, },
	{ 28800, 46, 98, 98, 95, },
	{ 19200, 71, 148, 148, 145, },
	{ 14400, 96, 197, 197, 194, },
	{ 9600, 146, 297, 297, 294, },
	{ 4800, 296, 595, 595, 592, },
	{ 2400, 592, 1189, 1189, 1186, },
	{ 1200, 1187, 2379, 2379, 2376, },
	{ 300, 4759, 9523, 9523, 9520, },
};

const int XMIT_START_ADJUSTMENT = 6;

#else
#error This version of SIM808SoftSerial supports only 20, 16 and 8MHz processors
#endif

SIM808Timer2Serial* SIM808Timer2Serial::_activeObject = 0;

SIM808Timer2Serial::SIM808Timer2Serial() :
	_rx_delay_centering(0),
	_rx_delay_intrabit(0),
	_rx_delay_stopbit(0),
	_tx_delay(0),
	cb(this){
	setTX();
	setRX();
#if ANDYDEBUGx
	Serial.println("Construct SoftwSerial function  ...");
#endif
}

int SIM808Timer2Serial::begin(long speed){
	_rx_delay_centering = _rx_delay_intrabit = _rx_delay_stopbit = _tx_delay = 0;
	
	for (unsigned i = 0; i < sizeof(table) / sizeof(table[0]); ++i){
		long baud = pgm_read_dword(&table[i].baud);
		if (baud == speed){
			_rx_delay_centering = pgm_read_word(&table[i].rx_delay_centering);
			_rx_delay_intrabit = pgm_read_word(&table[i].rx_delay_intrabit);
			_rx_delay_stopbit = pgm_read_word(&table[i].rx_delay_stopbit);
			_tx_delay = pgm_read_word(&table[i].tx_delay);
			break;
		}
	}
	if (_rx_delay_stopbit){
		if (digitalPinToPCICR(__RXPIN__)){
			*digitalPinToPCICR(__RXPIN__) |= _BV(digitalPinToPCICRbit(__RXPIN__));
			*digitalPinToPCMSK(__RXPIN__) |= _BV(digitalPinToPCMSKbit(__RXPIN__));
		}
		tunedDelay(_tx_delay);
	}
	_activeObject = this;
#if ANDYDEBUG
	Serial.println("Soft Serial1 start...");
	Serial.print("baud=");
	Serial.print(speed);
	Serial.print("  ");
	Serial.print("_rx_delay_centering=");
	Serial.print(_rx_delay_centering);
	Serial.print("  ");
	Serial.print("_rx_delay_intrabit=");
	Serial.print(_rx_delay_intrabit);
	Serial.print("  ");
	Serial.print("_rx_delay_stopbit=");
	Serial.print(_rx_delay_stopbit);
	Serial.print("  ");
	Serial.print("_tx_delay=");
	Serial.println(_tx_delay);
#endif
}	

void SIM808Timer2Serial::close(){
	_activeObject = 0;
}

void SIM808Timer2Serial::setTX(){
	pinMode(__TXPIN__,OUTPUT);
	digitalWrite(__TXPIN__,HIGH);

	_transmitBitMask = digitalPinToBitMask(__TXPIN__);
	uint8_t port = digitalPinToPort(__TXPIN__);
	_transmitPortRegister = portOutputRegister(port);
}

void SIM808Timer2Serial::setRX(){
	pinMode(__RXPIN__, INPUT);
	digitalWrite(__RXPIN__, HIGH);

	_receiveBitMask = digitalPinToBitMask(__RXPIN__);
	uint8_t port = digitalPinToPort(__RXPIN__);
	_receivePortRegister = portInputRegister(port);

	attachInterrupt(__RXINT__, SIM808Timer2Serial::handle_interrupt,FALLING);
}


void SIM808Timer2Serial::handle_interrupt(){
	if (_activeObject){
		_activeObject->Receive();//
	}
#if ANDYDEBUG
	else
	Serial.println("_activeObject = 0...");
#endif
}

void SIM808Timer2Serial::Receive(){
#if GCC_VERSION<40302
	asm volatile(
		"push r18 \n\t"
		"push r19 \n\t"
		"push r20 \n\t"
		"push r21 \n\t"
		"push r22 \n\t"
		"push r23 \n\t"
		"push r26 \n\t"
		"push r27 \n\t"
		::);
#endif

	int oldTail,thisHead;
	uint8_t dta=0;
	bool fullbuffer;
	bool morebytes = false;
	bool capturado_fullbuffer = 0;
	bool firstByte = true;
	int j=0;

	if (!rx_pin_read()){
		do{
			oldTail = cb.getTail();
			tunedDelay(_rx_delay_centering);
			fullbuffer = (cb.availableBytes()<6);
			if (fullbuffer&(!capturado_fullbuffer)){
				tx_pin_write(LOW);
			}

			for (uint8_t i = 0x1; i; i <<= 1){
				tunedDelay(_rx_delay_intrabit);
				uint8_t notify = ~i;
				if (rx_pin_read()){
					dta |= i;
				}else{
					dta &= notify;
				}
				if (fullbuffer&(!capturado_fullbuffer)){//
					if ((uint8_t)__XOFF__&i){
						tx_pin_write(HIGH);
					}else{
						tx_pin_write(LOW);
					}
				}
			}
			if (fullbuffer&(!capturado_fullbuffer)){
				tunedDelay(_rx_delay_intrabit);
				tx_pin_write(HIGH);
			}
			if (fullbuffer){
				capturado_fullbuffer = 1;
				_flags |= _SIM808SOFTSERIALFLAGS_SENTXOFF_;
			}
			if (!fullbuffer){
				tunedDelay(_rx_delay_stopbit);
			}
			cb.Write(dta);
			j++;
			
			/*if (keepThisChar(&dta)){
				cb.Write(dta);
				j++;*/
			if (firstByte){
				firstByte = false;
				thisHead = cb.getTail();
			}
			//}
			morebytes = false;
			for (int i = 0; i < __PARAGRAPHGUARD__; i++){//
				tunedDelay(1);
				if (!rx_pin_read()){
					morebytes = true;
					break;
				}
			}
		} while (morebytes);

		if (fullbuffer){
			if (mgr){
				mgr->manageMsg(thisHead,cb.getTail());
			}
		}else if (dta==10){//½ÓÊÕµ½0x0d 0x0a
				if (mgr){
				mgr->manageMsg(thisHead,cb.getTail());
			}
		}else if (dta == 32){
			if (mgr){
				mgr->manageMsg(thisHead,cb.getTail());
			}
		}
	}
#if GCC_VERSION < 40302
	asm volatile(
		"pop r27 \n\t"
		"pop r26 \n\t"
		"pop r23 \n\t"
		"pop r22 \n\t"
		"pop r21 \n\t"
		"pop r20 \n\t"
		"pop r19 \n\t"
		"pop r18 \n\t"
		::);
#endif
}

void SIM808Timer2Serial::spaceAvailable(){
	finalWrite((byte)__XON__);
}

void SIM808Timer2Serial::tunedDelay(uint16_t delay){
	uint8_t tmp = 0;
	asm volatile("sbiw    %0, 0x01 \n\t"
		"ldi %1, 0xFF \n\t"
		"cpi %A0, 0xFF \n\t"
		"cpc %B0, %1 \n\t"
		"brne .-10 \n\t"
		: "+r" (delay), "+a" (tmp)
		: "0" (delay)
		);
}

size_t SIM808Timer2Serial::write(uint8_t c){
	if (_tx_delay == 0){
		return 0;
	}/*
	if (c == 0x11){
		this->finalWrite(0x77);
		return this->finalWrite(0xee);
	}
	if (c == 0x13){
		this->finalWrite(0x77);
		return this->finalWrite(0xec);
	}
	if (c == 0x77){
		this->finalWrite(0x77);
		return this->finalWrite(0x88);
	}*/
	return this->finalWrite(c);
}

size_t SIM808Timer2Serial::writebyte(uint8_t c){
	if (_tx_delay == 0){
		return 0;
	}
	uint8_t oldSREG = SREG;
	cli();
	tx_pin_write(LOW);
	tunedDelay(_tx_delay + XMIT_START_ADJUSTMENT);

	for (byte mask = 0x01; mask; mask <<= 1){
		if (c&mask){
			tx_pin_write(HIGH);
		}else{
			tx_pin_write(LOW);
		}
		tunedDelay(_tx_delay);
	}
	tx_pin_write(HIGH);
	SREG = oldSREG;
	tunedDelay(_tx_delay);
#if ANDYDEBUG
	Serial.write(c);
#endif
}

size_t SIM808Timer2Serial::finalWrite(uint8_t c){
	uint8_t oldSREG = SREG;
	cli();
	tx_pin_write(LOW);
	tunedDelay(_tx_delay+XMIT_START_ADJUSTMENT);
	for (byte mask = 0x01; mask; mask <<= 1){
		if (c&mask){
			tx_pin_write(HIGH);
		}else{
			tx_pin_write(LOW);
		}
		tunedDelay(_tx_delay);
	}
	tx_pin_write(HIGH);
	SREG = oldSREG;
	tunedDelay(_tx_delay);
	
	return 1;
}

void SIM808Timer2Serial::tx_pin_write(uint8_t pste){
	if (pste == LOW){
		*_transmitPortRegister &= ~_transmitBitMask;
	}else{
		*_transmitPortRegister |= _transmitBitMask;
	}
}

uint8_t SIM808Timer2Serial::rx_pin_read(){
	return *_receivePortRegister & _receiveBitMask;
}

bool SIM808Timer2Serial::keepThisChar(uint8_t* c){
	if ((*c == 255) && (_flags & _SIM808SOFTSERIALFLAGS_SENTXOFF_))
	{
		_flags ^= _SIM808SOFTSERIALFLAGS_SENTXOFF_;
		return false;
	}

	// 0x77, w, is the escape character
	if (*c == 0x77)
	{
		_flags |= _SIM808SOFTSERIALFLAGS_ESCAPED_;
		return false;
	}

	// and these are the escaped codes
	if (_flags & _SIM808SOFTSERIALFLAGS_ESCAPED_)
	{
		if (*c == 0xEE)
			*c = 0x11;
		else if (*c == 0xEC)
			*c = 0x13;
		else if (*c == 0x88)
			*c = 0x77;

		_flags ^= _SIM808SOFTSERIALFLAGS_ESCAPED_;
		return true;
	}

	return true;
}

void SIM808Timer2SerialMgr::manageMsg(int from, int to){};


