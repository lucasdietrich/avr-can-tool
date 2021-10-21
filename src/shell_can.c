#include "shell_can.h"

#include "parser.h"

struct can_command
{
        char *rxtx;
        unsigned int id;
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

const struct cmd_descr can_command_descr[] PROGMEM = {
        CMD_DESCR(struct can_command, rxtx, CMD_TYPE_STRING),
        CMD_DESCR(struct can_command, id, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b0, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b1, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b2, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b3, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b4, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b5, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b6, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b7, CMD_TYPE_HEX),
};

// PROGMEM_STRING(rx_s, "rx");
PROGMEM_STRING(tx_s, "tx");

int8_t can_shell_handler(char *cmd, uint8_t len)
{
        int16_t args;
        int8_t ret;
        can_message_qi *p_msg = NULL;
        struct can_command data;

        /* parse arguments */
        args = cmd_parse(cmd, len, can_command_descr,
                ARRAY_SIZE(can_command_descr), &data);
        if (args <= 0 || !CMD_ARG_DEFINED(args, 0) || !CMD_ARG_DEFINED(args, 1)) {
                ret = (int8_t)args;
                goto exit;
        }

        /* allocate buffer */
        ret = can_msg_alloc(&p_msg, K_SECONDS(1));
        if (ret) {
                goto exit;
        }

        /* TX only supported now */
        ret = strcmp_P(data.rxtx, tx_s);
        if (ret) {
                ret = -1;
                goto exit;
        }

        /* set arbitration id */
        can_clear_message(&p_msg->msg);
        p_msg->msg.id = data.id;

        /* set buffer */
        for (uint_fast8_t i = 0; i < 8; i++) {
                if (CMD_ARG_DEFINED(args, 2 + i)) {
                        p_msg->msg.len++;
                        p_msg->msg.buffer[i] = (uint8_t)data.buffer[i];
                } else {
                        break;
                }
        }

        /* queue TX can message */
        can_tx_msg_queue(p_msg);

        return 0;

exit:
        can_msg_free(p_msg);
        return ret;
}