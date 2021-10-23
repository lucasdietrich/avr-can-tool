#include "shell_can.h"

#include "parser.h"

const struct cmd_descr can_command_descr[] PROGMEM = {
        CMD_DESCR(struct can_command, cmd, CMD_TYPE_STRING),
        CMD_DESCR(struct can_command, opt, CMD_TYPE_HEX),
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
PROGMEM_STRING(loopback_s, "loopback");

int8_t can_shell_handler(char *cmd, uint8_t len)
{
        int16_t args;
        struct can_command data;

        /* parse arguments */
        args = cmd_parse(cmd, len, can_command_descr,
                ARRAY_SIZE(can_command_descr), &data);
        if (args <= 0 || !CMD_ARG_DEFINED(args, 0)) {
                return (int8_t)args;
        }

        /* handle can command */
        if (strcmp_P(data.cmd, tx_s) == 0) {
                return can_handle_tx_command(&data, args);
        } else if (strcmp_P(data.cmd, loopback_s) == 0) {
                return can_handle_loopback_command(&data, args);
        }

        return -1;
}

int8_t can_handle_tx_command(struct can_command *data, int16_t args)
{
        int ret;
        can_message_qi *p_msg = NULL;

        if (!CMD_ARG_DEFINED(args, 1)) {
                return -1;
        }

        /* allocate buffer */
        ret = can_msg_alloc(&p_msg, K_SECONDS(1));
        if (ret) {
                goto exit;
        }

        /* set arbitration id */ 
        can_clear_message(&p_msg->msg);
        p_msg->msg.id = data->opt;

        /* set buffer */
        for (uint_fast8_t i = 0; i < 8; i++) {
                if (CMD_ARG_DEFINED(args, 2 + i)) {
                        p_msg->msg.len++;
                        p_msg->msg.buffer[i] = (uint8_t)data->buffer[i];
                } else {
                        break;
                }
        }

        /* queue can TX message */
        can_tx_msg_queue(p_msg);

        return 0;

exit:
        can_msg_free(p_msg);
        return ret;
}

int8_t can_handle_loopback_command(struct can_command *data, int16_t args)
{
        if (CMD_ARG_DEFINED(args, 1)) {
                can_cfg_set_loopback(data->opt);
        }

        PRINT_PROGMEM_STRING(s, "\n\tcan loopback = ");
        usart_u8(can_cfg_get_loopback());

        return 0;
}