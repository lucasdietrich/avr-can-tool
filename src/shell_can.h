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
                        unsigned int b0;
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

int8_t can_shell_handler(char *cmd, uint8_t len);

int8_t can_handle_tx_command(struct can_command *data, int16_t args);
int8_t can_handle_loopback_command(struct can_command *data, int16_t args);


#ifdef __cplusplus
}
#endif

#endif