#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cli/uart.h"


void blink_debug_led(void* param) {
    while(true) {
        uart::send_str("Hello, World!");
        PORTB |= (1<<PB5);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        PORTB &= ~(1<<PB5);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void init() {
    DDRB |= (1<<PB5);
    PORTB = 0x00;
    uart::init();
}

int main() {
    init();
    xTaskCreate(blink_debug_led, // task func
            "BLINK", // task name
            100, // max stack
            NULL, // no args
            tskIDLE_PRIORITY, // priority
            NULL); // task handle
    vTaskStartScheduler(); // runs forever
    return 0;
}
