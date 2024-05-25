#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cli/uart.h"
#include "cli/FreeRTOS_CLI.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TaskHandle_t read_and_respond_task = NULL;
TaskHandle_t blink_task = NULL;

// https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
void blink_debug_led_task(void* param) {
    while(true) {
        PORTB |= (1<<PB5);
        vTaskDelay(250 / portTICK_PERIOD_MS);
        PORTB &= ~(1<<PB5);
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

BaseType_t freemem(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    sprintf(pcWriteBuffer, "free heap %d bytes [%d bytes]\r\n", (int)xPortGetFreeHeapSize(), (int)xPortGetMinimumEverFreeHeapSize());
    return pdFALSE;
}

BaseType_t overhead(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    sprintf(pcWriteBuffer, "stack overhead %d bytes\r\n", (int)uxTaskGetStackHighWaterMark(read_and_respond_task));
    return pdFALSE;
}

BaseType_t spawn(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    signed char param1Len = 0;
    auto param1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &param1Len);
    BaseType_t res = pdFAIL;
    if(strcmp(param1, "blink") == 0) {
        if(blink_task) {
            sprintf(pcWriteBuffer, "already running\r\n");
            return pdFALSE;
        }
        res = xTaskCreate(blink_debug_led_task, "blink", 60, NULL, tskIDLE_PRIORITY, &blink_task);
    }
    if(res == pdPASS)
        sprintf(pcWriteBuffer, "SUCCESS\r\n");
    else
        sprintf(pcWriteBuffer, "ERROR [%d]\r\n", res);
    return pdFALSE;
}

BaseType_t kill(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString) {
    signed char param1Len = 0;
    auto param1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &param1Len);
    if(strcmp(param1, "blink") == 0)
        if(blink_task)
            vTaskDelete(blink_task);
    return pdFALSE;
}

void register_cmd(const CLI_Command_Definition_t& c) {
    auto res = FreeRTOS_CLIRegisterCommand(&c);
    if(res != pdPASS)
        uart::send_str("bad command registration\r\n");
}

void read_and_respond(void* param) {
    CLI_Command_Definition_t freemem_cmd{
        .pcCommand="freemem",
        .pcHelpString="freemem:\r\n list total memory left\r\n\r\n",
        .pxCommandInterpreter=freemem,
        .cExpectedNumberOfParameters=0
    };
    register_cmd(freemem_cmd);
    CLI_Command_Definition_t spawn_cmd{
        .pcCommand="spawn",
        .pcHelpString="spawn <blink>:\r\n spawn a task, available tasks: [blink]\r\n\r\n",
        .pxCommandInterpreter=spawn,
        .cExpectedNumberOfParameters=1
    };
    register_cmd(spawn_cmd);
    CLI_Command_Definition_t kill_cmd{
        .pcCommand="kill",
        .pcHelpString="kill <blink>:\r\n kill a task, available tasks: [blink]\r\n\r\n",
        .pxCommandInterpreter=kill,
        .cExpectedNumberOfParameters=1
    };
    register_cmd(kill_cmd);

#define INBUF_SIZE 16
#define OUTBUF_SIZE 128
    char inbuf[INBUF_SIZE];
    memset(inbuf, 0x00, INBUF_SIZE);
    char outbuf[OUTBUF_SIZE];
    memset(outbuf, 0x00, INBUF_SIZE);
    unsigned int n;
    BaseType_t xReturned;
    uart::send_str("Welcome to FreeRTOS on Arduino Uno!\r\n");
    freemem(outbuf, OUTBUF_SIZE, "");
    uart::send_str(outbuf);
    overhead(outbuf, OUTBUF_SIZE, "");
    uart::send_str(outbuf);
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
    xTaskCreate(read_and_respond,
            "Console",
            650,
            NULL,
            tskIDLE_PRIORITY,
            &read_and_respond_task);
    vTaskStartScheduler(); // runs forever
    return 0;
}
