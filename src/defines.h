#ifndef _DEFINES_H
#define _DEFINES_H


#define K_MODULE_APP    0x20
#define K_MODULE_CAN    0x21
#define K_MODULE_SHELL  0x22


#define PROGMEM_STRING(name, string)            \
    static const char name[] PROGMEM = string

#define PRINT_PROGMEM_STRING(name, string)      \
    static const char name[] PROGMEM = string;  \
    usart_print_p(name)
    
    
#endif /* _DEFINES_H */