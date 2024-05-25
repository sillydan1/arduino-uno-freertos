#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cli/uart.h"
#include "cli/FreeRTOS_CLI.h"
#include <stdio.h>
#include <string.h>

TaskHandle_t read_and_respond_task;
TaskHandle_t blink_task;

// https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
void blink_debug_led(void* param) {
    while(true) {
        PORTB |= (1<<PB5);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        PORTB &= ~(1<<PB5);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

BaseType_t freemem(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    sprintf(pcWriteBuffer, "free heap %d bytes [%d bytes]\r\n", (int)xPortGetFreeHeapSize(), (int)xPortGetMinimumEverFreeHeapSize());
    return pdFALSE; // no more stuff to do
}

BaseType_t loopmem(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    auto m = uxTaskGetStackHighWaterMark(read_and_respond_task);
    sprintf(pcWriteBuffer, "cli headroom %d bytes\r\n", (int)m);
    return pdFALSE; // no more stuff to do
}

BaseType_t blinkmem(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    auto m = uxTaskGetStackHighWaterMark(blink_task);
    sprintf(pcWriteBuffer, "blink headroom %d bytes\r\n", (int)m);
    return pdFALSE; // no more stuff to do
}

void register_cmd(const CLI_Command_Definition_t& c) {
    auto res = FreeRTOS_CLIRegisterCommand(&c);
    if(res != pdPASS)
        uart::send_str("bad command registration\r\n");
}

void read_and_respond(void* param) {
    CLI_Command_Definition_t freemem_cmd{
        .pcCommand="freemem",
        .pcHelpString="freemem:\r\n print free memory\r\n\r\n",
        .pxCommandInterpreter=freemem,
        .cExpectedNumberOfParameters=0
    };
    CLI_Command_Definition_t loopmem_cmd{
        .pcCommand="loopmem",
        .pcHelpString="loopmem:\r\n print memory headroom\r\n\r\n",
        .pxCommandInterpreter=loopmem,
        .cExpectedNumberOfParameters=0
    };
    CLI_Command_Definition_t blinkmem_cmd{
        .pcCommand="blinkmem",
        .pcHelpString="blinkmem:\r\n print memory headroom\r\n\r\n",
        .pxCommandInterpreter=blinkmem,
        .cExpectedNumberOfParameters=0
    };
    register_cmd(freemem_cmd);
    register_cmd(loopmem_cmd);
    register_cmd(blinkmem_cmd);
#define INBUF_SIZE 64
#define OUTBUF_SIZE 128
    char inbuf[INBUF_SIZE];
    char outbuf[OUTBUF_SIZE];
    unsigned int n;
    BaseType_t xReturned;
    while(true) {
        uart::send_str("> ");
        n = uart::get_line(inbuf, INBUF_SIZE-1);
        uart::send_str("\r\n");
        if(n == 0)
            continue;
        inbuf[n] = '\0';
        do {
            xReturned = FreeRTOS_CLIProcessCommand(inbuf, outbuf, OUTBUF_SIZE-1);
            outbuf[OUTBUF_SIZE-1] = '\0';
            uart::send_str(outbuf);
        } while(xReturned != pdFALSE);
        memset(inbuf, 0x00, INBUF_SIZE);
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
            60,
            NULL,
            tskIDLE_PRIORITY,
            &blink_task);
    xTaskCreate(read_and_respond,
            "Console",
            512,
            NULL,
            tskIDLE_PRIORITY,
            &read_and_respond_task);
    vTaskStartScheduler(); // runs forever
    return 0;
}
