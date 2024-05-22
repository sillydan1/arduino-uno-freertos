#include <avr/io.h>
#include "uart.h"

// For reference: (section 19)
// https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
namespace uart {
	// TODO: ISR handlers, so we dont stall the task(s)
	// TODO: Receive
	// TODO: Consider doing this as a class
	// TODO: Receive and send are not cooperative and stalls the scheduler.
	void init() {
		UBRR0H = UBRRH_VALUE;
		UBRR0L = UBRRL_VALUE;
		// UCSR0B = _BV(RXEN0) | _BV(TXEN0);
		UCSR0B = _BV(TXEN0);
		// Frame format: 8 data, 1 stop
		UCSR0C = _BV(USBS0) | _BV(UCSZ00) | _BV(UCSZ01);
	}

	void send_char(char c) {
		while(!(UCSR0A & _BV(UDRE0))) ;
		UDR0 = c;
	}

	void send_str(const char* str) {
		unsigned int i = 0;
		while(str[i])
			send_char(str[i++]);
	}

	void send_strn(const char* str, unsigned int len) {
		for(unsigned int i = 0; i < len; i++)
			send_char(str[i]);
	}

	unsigned int get_line(char* buf, unsigned int max) {
		int i = 0;
		while(char c = get_char()) {
			if(c == '\n')
				break;
			if(i >= max)
				break;
			buf[i++] = c;
		}
		return i;
	}

	char get_char() {
		while(!(UCSR0A & _BV(RXC0))) ;
		return UDR0;
	}
}
