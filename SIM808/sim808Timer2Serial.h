#ifndef __SIM808TIMER2SERIAL__
#define __SIM808TIMER2SERIAL__

#include <sim808CircularBuffer.h>
#include <avr/pgmspace.h>
#include <debugDF.h>
#include <inttypes.h>

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

class SIM808Timer2SerialMgr{
	public:
		virtual void manageMsg(int from, int to);
};


class SIM808Timer2Serial :public SIM808CircularBufferManager{
	private:
		uint8_t _receiveBitMask;
		volatile uint8_t *_receivePortRegister;
		uint8_t _transmitBitMask;
		volatile uint8_t *_transmitPortRegister;

		static SIM808Timer2Serial* _activeObject;
		SIM808Timer2SerialMgr* mgr;

		uint16_t _rx_delay_centering;
		uint16_t _rx_delay_intrabit;
		uint16_t _rx_delay_stopbit;
		uint16_t _tx_delay;
		uint8_t _flags;
		uint8_t olddta;

		void tx_pin_write(uint8_t pin_state);
		uint8_t rx_pin_read();
		void Receive();

		void setTX();
		void setRX();

		void setComsReceived();
		virtual size_t finalWrite(uint8_t);
		bool keepThisChar(uint8_t* c);
	public:
		static void tunedDelay(uint16_t delay);
		SIM808CircularBuffer cb;
		inline void registerMgr(SIM808Timer2SerialMgr* manager){ mgr = manager; };
		void spaceAvailable();
		virtual size_t write(uint8_t);//write a character in serial connection
		size_t writebyte(uint8_t c);
		SIM808Timer2Serial();
		int begin(long speed);
		static inline void handle_interrupt();
		void close();
};

#endif