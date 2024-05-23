#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cli/uart.h"
#include "cli/FreeRTOS_CLI.h"
#include <stdio.h>
#include <string.h>

// https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
void blink_debug_led(void* param) {
    while(true) {
        PORTB |= (1<<PB5);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        PORTB &= ~(1<<PB5);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

BaseType_t freemem(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    sprintf(pcWriteBuffer, "free heap %d bytes [%d bytes]\r\n", (int)xPortGetFreeHeapSize(), (int)xPortGetMinimumEverFreeHeapSize());
    return pdFALSE; // no more stuff to do
}

void read_and_respond(void* param) {
    CLI_Command_Definition_t greeter{
        .pcCommand="freemem",
        .pcHelpString="freemem:\r\n print free memory\r\n",
        .pxCommandInterpreter=freemem,
        .cExpectedNumberOfParameters=0
    };
    auto res = FreeRTOS_CLIRegisterCommand(&greeter);
    if(res != pdPASS) {
        uart::send_str("bad command registration\r\n");
        return;
    }
    char inbuf[32];
    char outbuf[128];
    unsigned int n;
    BaseType_t xReturned;
    while(true) {
        uart::send_str("> ");
        n = uart::get_line(inbuf, 31);
        inbuf[n] = '\0';
        do {
            xReturned = FreeRTOS_CLIProcessCommand(inbuf, outbuf, 127);
            outbuf[127] = '\0';
            uart::send_str(outbuf);
        } while(xReturned != pdFALSE);
        memset(inbuf, 0x00, 32);
    }
}

void init() {
    DDRB |= (1<<PB5);
    PORTB = 0x00;
    uart::init();
}

int main() {
    init();
    xTaskCreate(blink_debug_led,
            "BLINK",
            100,
            NULL,
            tskIDLE_PRIORITY,
            NULL);
    xTaskCreate(read_and_respond,
            "Console",
            256,
            NULL,
            tskIDLE_PRIORITY,
            NULL);
    vTaskStartScheduler(); // runs forever
    return 0;
}
