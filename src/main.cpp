#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cli/uart.h"


void blink_debug_led(void* param) {
    while(true) {
        PORTB |= (1<<PB5);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        PORTB &= ~(1<<PB5);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void read_and_respond(void* param) {
    char buf[32];
    while(true) {
        uart::send_str("> ");
        auto n = uart::get_line(buf, 31);
        buf[n] = '\0';
        uart::send_str("Hello, ");
        uart::send_str(buf);
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
