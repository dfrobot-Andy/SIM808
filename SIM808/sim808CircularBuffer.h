#ifndef __SIM808CIRCULARBUFFER__
#define __SIM808CIRCULARBUFFER__

#include <inttypes.h>
#include <stddef.h>
#include <debugDF.h>

#ifndef byte
#define byte uint8_t
#endif

#define __BUFFERSIZE__ 512//512
#define __BUFFERMASK__ 0x1FF

class SIM808CircularBufferManager{
	public:
		
		virtual void spaceAvailable();//if there is space available in the buffer,we can receive data in timer2 manage function
};

class SIM808CircularBuffer{
	private:
		volatile int head;		//first written one
		volatile int tail;		//last written one

		SIM808CircularBufferManager* cbm;		//circular buffer manager

		volatile byte theBuffer[__BUFFERSIZE__];
		/* Checks if a substring exists in the buffer
		@param reference	Substring
		@param thishead		Head
		@param thistail		Tail
		@param from			Initial byte position
		@param to			Final byte position
		@return true if exists, in otherwise return false
		*///
		bool locate(const char* reference, int thishead, int thistail, int* from = 0, int* to = 0);
		bool locate(const char* reference, int thishead, int thistail, uint8_t len);
	public:
		SIM808CircularBuffer(SIM808CircularBufferManager*	mgr=0); //circular
		inline int availableBytes(){ return ((head - (tail + 1))&__BUFFERMASK__); };
		inline int storedBytes(){ return ((tail - head)&__BUFFERMASK__); };
		
		int Write(char c);//write acharacter in circular buffer
		char Read();//read a character and moves the pointer
		char Peek(int increment);//read a character but does not move the pointer
		inline int* firstString(){ return (int*)theBuffer + head; };//returns a pointer to the head of the buffer
		char* nextString();//go forward one string
		void Flush();//flush circular buffer
		inline int getTail(){ return tail; };//get tail
		inline int getHead(){ return head; };//get head
		uint8_t getOldDta();
		inline void deleteToTheEnd(int from){ tail = from; };//delete circular buffer to the end
		bool locate(const char* reference, uint8_t len);//Checks if a substring exists in the buffer
		bool locate(const char* reference);
		bool chopUntil(const char* reference, bool movetotheend, bool head=true);// Locates reference. If found, moves head (or tail) to the beginning (or end)
		int readInt();//Reads an integer from the head. Stops with first non blank, non number character

		bool extractSubString(const char* from, const char* to, char* buffer, int bufsize);//Extract a substring from circular buffer´Óbuffer
		bool retrieveBuffer(char* buffer, int bufsize, int& SizeWritten);//Retrieve all the contents of buffer from head to tail
	//	void debugBuffer();
		static void printCharDebug(uint8_t c);
};


#endif