#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

void blink_debug_led(void* param) {
    while(true) {
        PORTB |= (1<<PB5);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        PORTB &= ~(1<<PB5);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void init() {
    DDRB |= (1<<PB5);
    PORTB = 0x00;
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
