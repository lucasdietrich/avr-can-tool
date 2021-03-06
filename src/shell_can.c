#include "shell_can.h"

#include "parser.h"

/* as defined in "mcp2515_can_dfs.h" */
#define CAN_EXTID (1)
#define CAN_STDID (0)

#define CAN_MAX_EXTID   (0x1FFFFFFF)
#define CAN_MAX_STDID   (0x07FF)

/*___________________________________________________________________________*/

static int8_t build_can_message(struct can_command *data, int16_t args, bool rtr)
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
        p_msg->msg.id = data->opt & CAN_MAX_EXTID; /* max extended ID */
        
        /* set standart or extended can message type */
        p_msg->msg.isext = p_msg->msg.id > CAN_MAX_STDID ? CAN_EXTID : CAN_STDID;

        if (rtr) {
                /* requested len */
                if (CMD_ARG_DEFINED(args, 2) && (data->rtr_len <= 8)) {
                        p_msg->msg.len = data->rtr_len;
                }
        } else {
                /* set buffer */
                for (uint_fast8_t i = 0; i < 8; i++) {
                        if (CMD_ARG_DEFINED(args, 2 + i)) {
                                p_msg->msg.len++;
                                p_msg->msg.buf[i] = (uint8_t)data->buffer[i];
                        } else {
                                break;
                        }
                }
        }

        /* data message */
        p_msg->msg.rtr = (int8_t)rtr;

        /* queue can TX message */
        can_tx_msg_queue(p_msg);

        return 0;

exit:
        can_msg_free(p_msg);
        return ret;
}

static int8_t can_handle_tx_command(struct can_command *data, int16_t args)
{
        return build_can_message(data, args, false);
}

static int8_t can_handle_rtr_command(struct can_command *data, int16_t args)
{
        return build_can_message(data, args, true);
}

static int8_t can_handle_rx_command(struct can_command *data, int16_t args)
{
        if (CMD_ARG_DEFINED(args, 1)) {
                can_cfg_set_rx(data->opt);
        }

        printf_P(PSTR("\n\tcan rx = %02hhx"), can_cfg_get_rx());

        return 0;
}

static int8_t can_handle_int_command(struct can_command *data, int16_t args)
{
        if (CMD_ARG_DEFINED(args, 1)) {
                can_cfg_set_int(data->opt);
        }

        printf_P(PSTR("\n\tcan int = %02hhx"), can_cfg_get_int());

        return 0;
}

static int8_t can_handle_loopback_command(struct can_command *data, int16_t args)
{
        if (CMD_ARG_DEFINED(args, 1)) {
                can_cfg_set_loopback(data->opt);
        }

        printf_P(PSTR("\n\tcan loopback = %02hhx"), can_cfg_get_loopback());

        return 0;
}

/*___________________________________________________________________________*/

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

int can_shell_handler(char *cmd, uint8_t len)
{
        int16_t args;
        struct can_command data;

        /* parse arguments */
        args = cmd_parse(cmd, len, can_command_descr,
                ARRAY_SIZE(can_command_descr), &data);
        if (args <= 0 || !CMD_ARG_DEFINED(args, 0)) {
                return args;
        }

        /* handle can command */
        if (strcmp_P(data.cmd, PSTR("tx")) == 0) {
                return can_handle_tx_command(&data, args);
        } else if (strcmp_P(data.cmd, PSTR("rx")) == 0) {
                return can_handle_rx_command(&data, args);
        } else if (strcmp_P(data.cmd, PSTR("int")) == 0) {
                return can_handle_int_command(&data, args);
        } else if (strcmp_P(data.cmd, PSTR("loopback")) == 0) {
                return can_handle_loopback_command(&data, args);
        } else if (strcmp_P(data.cmd, PSTR("rtr")) == 0) {
                return can_handle_rtr_command(&data, args);
        }
        return -1;
}