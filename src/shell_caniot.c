#include "shell_caniot.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#include <avrtos/kernel.h>
#include <caniot.h>

// discover

// device
// device set DID
// device unset

// readattr 0x100
// writeattr 0x100 0x200
// telemetry
// commmand EP 1 2 3 4 5 6 7 8

#define SUBCMD_DEVICE "device"
#define SUBCMD_DEVICE_LEN (sizeof(SUBCMD_DEVICE) - 1)

#define SUBCMD_SET "set"
#define SUBCMD_SET_LEN (sizeof(SUBCMD_SET) - 1)

#define SUBCMD_UNSET "unset"
#define SUBCMD_UNSET_LEN (sizeof(SUBCMD_UNSET) - 1)

static union deviceid cur_device = CANIOT_DEVICE_BROADCAST;

static bool is_device_api(char *cmd)
{
        return strncmp_P(cmd, PSTR(SUBCMD_DEVICE), SUBCMD_DEVICE_LEN) == 0;
}

static int device_api_handler(char *cmd, uint8_t len)
{
        if (len == 0) {
                if (caniot_is_broadcast(cur_device)) {
                        printf_P(PSTR(" : BROADCAST"));
                } else {
                        printf_P(PSTR(" : %d"), cur_device);
                }
        } else if (strncmp_P(cmd, PSTR(SUBCMD_UNSET), SUBCMD_UNSET_LEN) == 0) {
                cur_device = CANIOT_DEVICE_BROADCAST;
        } else if (strncmp_P(cmd, PSTR(SUBCMD_SET), SUBCMD_SET_LEN) == 0) {
                if (len == 1) {
                        printf_P(PSTR(" : missing argument"));
                } else {
                        union deviceid did;
                        if (sscanf_P(cmd + SUBCMD_SET_LEN, PSTR("%hhd"), &did.val) == 1) {
                                if (caniot_valid_deviceid(did)) {
                                        cur_device = did;
                                } else {
                                        printf_P(PSTR(" : invalid device id"));
                                }                                
                        } else {
                                printf_P(PSTR(" : invalid argument"));
                                return -CANIOT_EDEVICE;
                        }
                }
        } else {
                printf_P(PSTR(" : invalid subcommand\n"));
                return -CANIOT_EDEVICE;
        }

        return 0;
}

static int caniot_discover(void)
{
        return -CANIOT_ENIMPL;
}

static int caniot_telemetry(void)
{
        return -CANIOT_ENIMPL;
}

int shell_caniot_handler(char *cmd, uint8_t len)
{
        int ret = -EINVAL;

        if (is_device_api(cmd)) {
                ret = device_api_handler(cmd + sizeof(SUBCMD_DEVICE),
                                         len <= sizeof(SUBCMD_DEVICE) ?
                                         0 : len - sizeof(SUBCMD_DEVICE));
        } else if (strncmp_P(cmd, PSTR("discover"), sizeof("discover") - 1) == 0) {
                ret = caniot_discover();
        } else if (strncmp_P(cmd, PSTR("telemetry"), sizeof("telemetry") - 1) == 0) {
                ret = caniot_telemetry();
        } else if (strncmp_P(cmd, PSTR("command"), sizeof("command") - 1) == 0) {
                // ret = caniot_command(cur_device, cmd + sizeof("command"), len - sizeof("command"));
        } else {
                printf_P(PSTR(" : invalid command\n"));
                ret = -EINVAL;
        }

        return ret;
}