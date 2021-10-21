#ifndef _PARSER_H
#define _PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <avrtos/kernel.h>

#include "app.h"

/*___________________________________________________________________________*/

struct cmd_descr {
        uint8_t type; /* also include type len */
        uint8_t offset;
};

#define CMD_TYPE_STRING         0
#define CMD_TYPE_NUMBER         1
#define CMD_TYPE_HEX            2

#define CMD_DESCR(struct, field_name, field_type)                       \
        {                                                               \
                .type = field_type,                                     \
                .offset = offsetof(struct, field_name)                  \
        }

#define CMD_ARG_DEFINED(ret, arg_n) TEST_BIT(ret, 1 << (arg_n))

/*___________________________________________________________________________*/

int16_t cmd_parse(char *buffer, uint16_t len,
        const struct cmd_descr *descr, uint8_t descr_size, void *data);

int16_t cmd_parse_arg(char *buffer, uint16_t len,
        const uint8_t type, void *const data_addr);

int16_t cmd_parse_string(char* buffer, uint16_t len);

int16_t cmd_parse_number(char* buffer, uint16_t len, uint32_t* number);

int16_t cmd_parse_hex(char* buffer, uint16_t len, uint32_t* number);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif