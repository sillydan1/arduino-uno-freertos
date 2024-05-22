#ifndef CLI_UART_H
#define CLI_UART_H
#include <util/setbaud.h>

namespace uart {
    void init();
    void send_char(char character);
    void send_str(const char* str);
    void send_strn(const char* str, unsigned int len);
    unsigned int get_line(char* buf, unsigned int max);
    char get_char();
}

#endif
