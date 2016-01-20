#include <sim808CircularBuffer.h>
#include <HardwareSerial.h>
#include <Arduino.h>

void SIM808CircularBufferManager::spaceAvailable(){ return; };

SIM808CircularBuffer::SIM808CircularBuffer(SIM808CircularBufferManager*	mgr){
	head = 0;
	tail = 0;
	cbm = mgr;
}

int SIM808CircularBuffer::Write(char c){
	int aux = (tail + 1)&__BUFFERMASK__;
	if (aux != head){
		theBuffer[tail] = c;
		theBuffer[aux] = 0;
		tail = aux;
		return 1;
	}
	else{
		return 0;
	}
}

char SIM808CircularBuffer::Read(){
	char res;
	if (head != tail){
		res = theBuffer[head];
		head = (head + 1)& __BUFFERMASK__;
		return res;
	}else{
		return 0;
	}
}

char SIM808CircularBuffer::Peek(int increment){
	char res;
	int num_aux;

	if (tail > head){
		num_aux = tail - head;
	}else{
		num_aux = 1024 - head + tail;
	}

	if (increment < num_aux){
		res = theBuffer[head];
		return res;
	}else{
		return 0;
	}
}

void SIM808CircularBuffer::Flush(){
	head = tail;
}

char* SIM808CircularBuffer::nextString(){
	while (head != tail){
		head = (head + 1) & __BUFFERMASK__;
		if (theBuffer[head] == 0){
			head = (head + 1) & __BUFFERMASK__;
			return (char*)theBuffer + head;
		}
	}
	return 0;
}

bool SIM808CircularBuffer::locate(const char* reference){
	return locate(reference, head, tail, 0, 0);
}

bool SIM808CircularBuffer::locate(const char* reference,uint8_t len){
	return locate(reference, head, tail, len);
}

bool SIM808CircularBuffer::chopUntil(const char* reference, bool movetotheend, bool usehead){
	int from, to;

	if (locate(reference, head, tail, &from, &to)){
		if (usehead){
			if (movetotheend)	head = (to + 1) & __BUFFERMASK__;
			else  head = from;
		}else{
			if (movetotheend)	tail = (to + 1) & __BUFFERMASK__;
			else tail = from;
		}
		return true;
	}else{
		return false;
	}
}

bool SIM808CircularBuffer::locate(const char* reference, int thishead, int thistail, int* from, int* to){
	int refcursor = 0;
	bool into = false;
	int b2, binit;
	bool possible = 1;

	if (reference[0] == 0)	return true;

	for (int b1 = thishead; b1 != thistail; b1 = (b1 + 1)&__BUFFERMASK__){
		possible = 1;
		b2 = b1;
		while (possible && (b2 != thistail)){
			if (theBuffer[b2] == reference[refcursor]){
				if (!into) binit = b2;
				into = true;
				refcursor++;
				if (reference[refcursor] == 0){
					if (from) *from = binit;
					if (to) *to = b2;
					return true;
				}
			}else if (into == true){
				possible = 0;
				into = false;
				refcursor = 0;
			}
			b2 = (b2 + 1)& __BUFFERMASK__;
		}
	}
	return false;
}

bool SIM808CircularBuffer::locate(const char* reference, int thishead, int thistail, uint8_t len){
	int refcursor = 0;
	bool into = false;
	int b2, binit;
	bool possible = 1;
	uint8_t index = 0;
	if (reference[0] == 0)	return true;
	for (int b1 = thishead; b1 != thistail; b1 = (b1 + 1)&__BUFFERMASK__){
		possible = 1;
		b2 = b1;
		while (possible && (b2 != thistail)){
			if (theBuffer[b2] == reference[refcursor]){
				if (!into) binit = b2;
				into = true;
				refcursor++;
				index++;
				if (index==len){				
					return true;
				}
			}
			else if (into == true){
				possible = 0;
				into = false;
				refcursor = 0;
			}
			b2 = (b2 + 1)& __BUFFERMASK__;
		}
	}
	
	return false;
}

bool SIM808CircularBuffer::extractSubString(const char* from, const char* to, char* buffer, int bufsize){
	int t1, t2, b, i;

	if (!locate(from, head, tail, 0, &t1))	return false;

	if (!locate(to, t1, tail, &t2, 0))
		return false;

	for (i = 0, b = t1-6; i<bufsize, b != ((t2)& __BUFFERMASK__); i++, b = (b + 1)& __BUFFERMASK__)
		buffer[i] = theBuffer[b];
	buffer[i] = 0;

	return true;
}

int SIM808CircularBuffer::readInt(){
	int res = 0;
	byte c;
	bool anyfound = false;
	bool negative = false;

	for (int b = head + 1; b != tail; b = (b + 1)& __BUFFERMASK__){
		c = theBuffer[b];
		if ((c == ' ') && (!anyfound)){
		}else if ((c == '-') && (!anyfound)){
			negative = true;
			anyfound = true;  // Don't admit blanks after -
		}else if ((c >= '0') && (c <= '9')){
			anyfound = true;
			res = (res * 10) + (int)c - 48;
		}else{
			if (negative)	res = (-1)*res;
			return res;
		}
	}
	if (negative)	res = (-1)*res;

	return res;
}
/*
void SIM808CircularBuffer::debugBuffer()
{
	int h1 = head;
	int t1 = tail;

	Serial.println();
	Serial.print(h1);
	Serial.print(" ");
	Serial.print(t1);
	Serial.print('>');
	for (int b = h1; b != t1; b = (b + 1)& __BUFFERMASK__)
		printCharDebug(theBuffer[b]);
	Serial.println();
}
*/

void SIM808CircularBuffer::printCharDebug(uint8_t c)
{
	Serial.write(c);
}

bool SIM808CircularBuffer::retrieveBuffer(char* buffer, int bufsize, int& SizeWritten){
	int i, b;

	b = head;
	for (i = 0; i<bufsize; i++){
		if (b != tail){
			buffer[i] = theBuffer[b];
			buffer[i + 1] = 0;
			b = (b + 1)& __BUFFERMASK__;
			SizeWritten = i + 1;
		}
	}

	return true;
}


uint8_t SIM808CircularBuffer::getOldDta(){
	uint8_t req;
	req = theBuffer[tail];
	return req;// (uint8_t*)theBuffer(tail);
}


