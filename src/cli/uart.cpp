#include <avr/io.h>
#include "uart.h"
#include <avr/interrupt.h>
#include <portmacro.h>
#include "FreeRTOS.h"
#include "queue.h"

// For reference: (section 19)
// https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
namespace uart {
	static QueueHandle_t rx_buf;
	static QueueHandle_t tx_buf;

	ISR(USART_RX_vect) {
		char c = UDR0;
		auto taskWoken = pdFALSE;
		xQueueGenericSendFromISR(rx_buf, &c, &taskWoken, queueSEND_TO_BACK);
		// xQueueGenericSendFromISR(tx_buf, &c, nullptr, queueSEND_TO_BACK); // Echo ON
		send_char(c);
		if(taskWoken != pdFALSE)
			portYIELD_FROM_ISR();
	}
	// BOOKMARK: FreeRTOS/Demo/Common/Minimal/comtest.c
	//		     as well as FreeRTOS/Demo/AVR_ATMega4809_Atmel_Studio/RTOSDemo/serial/serial.c

	// TODO: Consider doing this as a class
	// TODO: Receive and send are not cooperative and stalls the scheduler.
	void init() {
		portENTER_CRITICAL();
		{
			unsigned int bufferLength = 64;
			rx_buf = xQueueCreate(bufferLength, sizeof(char));
			tx_buf = xQueueCreate(bufferLength, sizeof(char));
			UBRR0H = UBRRH_VALUE;
			UBRR0L = UBRRL_VALUE;
			UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);
			UCSR0C = _BV(USBS0) | _BV(UCSZ00) | _BV(UCSZ01);
		}
		portEXIT_CRITICAL();
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

	// TODO: should return a tuple <false,!invalid> <true,result> - rusty
	char get_char(unsigned int timeout_ms) {
		char result;
		if(xQueueReceive(rx_buf, &result, timeout_ms / portTICK_PERIOD_MS))
			return result;
		return '\0';
	}

	unsigned int get_line(char* buf, unsigned int max) {
		unsigned int i = 0;
		while(i < max) {
			char c;
			if(xQueueReceive(rx_buf, &c, 100 / portTICK_PERIOD_MS)) {
				if(c == '\r')
					break;
				buf[i++] = c;
			}
		}
		return i;
	}
}
