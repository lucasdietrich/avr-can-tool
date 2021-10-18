#include "parser.h"

static int16_t find(const char* buffer, uint16_t len, char c)
{
        for (uint_fast16_t i = 0; i < len; i++) {
                if (buffer[i] == c)
                        return i;
                else if (buffer[i] == '\0')
                        break;
        }
        return -1;
}

int16_t cmd_parse_string(char* buffer, uint16_t len)
{
        uint16_t i = 0;

        while (i < len) {
                const char c = buffer[i];
                switch (c) {
                case '\0':
                case ' ':
                        return i;
                default:
                        if (IN_RANGE(c, 'a', 'z') || (c == '-')) {
                                break;
                        } else {
                                return -1;
                        }
                }
                i++;
        }
        return i;
}

int16_t cmd_parse_hex(char* buffer, uint16_t len, uint32_t* number)
{
        return sscanf(buffer, "%lx", number);
}

int16_t cmd_parse_number(char* buffer, uint16_t len, uint32_t* number)
{
        return sscanf(buffer, "%lu", number);
}

int16_t cmd_parse_arg(char *buffer, uint16_t len,
        const uint8_t type, void *const data_addr)
{
        int16_t ret;

        switch (type) {
        case CMD_TYPE_STRING:
                ret = cmd_parse_string(buffer, len);
                if (ret > 0) {
                        *(char **)data_addr = buffer;
                        buffer[ret] = '\0';
                        return ret + 1;
                }
                break;

        case CMD_TYPE_NUMBER:
        case CMD_TYPE_HEX:
                ret = (type == CMD_TYPE_NUMBER)
                        ? cmd_parse_number(buffer, len, (uint32_t *)data_addr)
                        : cmd_parse_hex(buffer, len, (uint32_t *)data_addr);
                if (ret == 1u) {
                        ret = find(buffer, len, ' ');
                        if (ret >= 0)
                                return ret + 1;
                        else
                                return len;
                }
                break;
        }
        return -1;
}

int16_t cmd_parse(char *buffer, uint16_t len,
        const struct cmd_descr *descr, uint8_t descr_size, void *data)
{
        int16_t parsed, ret;
        uint8_t arg_n = 0;
        uint16_t i = 0;

        while ((arg_n < descr_size) && (i < len)) {
                const uint8_t type = pgm_read_byte(&descr[arg_n].type);
                const uint8_t offset = pgm_read_byte(&descr[arg_n].offset);

                parsed = cmd_parse_arg(&buffer[i], len - i, type, data + offset);
                if (parsed <= 0)
                        return -1;
                else
                        i += parsed;

                SET_BIT(ret, 1 << arg_n++);
        }
        
        return ret;
}