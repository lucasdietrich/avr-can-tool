#ifndef _SHELL_CAN_H
#define _SHELL_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can.h"

struct can_command
{
        char *cmd;
        long unsigned int opt;
        union
        {
                struct {
                        union {
                                unsigned int b0;
                                unsigned int rtr_len;
                        };
                        unsigned int b1;
                        unsigned int b2;
                        unsigned int b3;
                        unsigned int b4;
                        unsigned int b5;
                        unsigned int b6;
                        unsigned int b7;
                };
                unsigned int buffer[8];
        };


};

int can_shell_handler(char *cmd, uint8_t len);


#ifdef __cplusplus
}
#endif

#endif