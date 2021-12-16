#include "caniot_controller.h"

#include <caniot.h>
#include <controller.h>

#include <avrtos/prng.h>

#include "osspec.h"
#include "can.h"

#if defined(CONFIG_CANIOT_LIB)

static int frame_caniot2can(struct caniot_frame *frame, can_message *msg)
{
        if (!frame || !msg) {
                return -EINVAL;
        }

        memcpy(msg->buf, frame->buf, frame->len);
        msg->len = frame->len;
        msg->std = frame->id.raw;
        msg->rtr = 0;
        msg->ext = 0;

        return 0;
}

static int frame_can2caniot(can_message *msg, struct caniot_frame *frame)
{
        if (!frame || !msg) {
                return -EINVAL;
        }

        if (msg->ext || msg->rtr) {
                return -EINVAL;
        }

        memcpy(frame->buf, msg->buf, msg->len);
        frame->len = msg->len;
        frame->id.raw = msg->std;

        return 0;
}

static int send(struct caniot_frame *frame, uint32_t delay)
{
        ARG_UNUSED(delay);

        can_message_qi *msg;

        int8_t ret = can_msg_alloc(&msg, K_FOREVER);
        if (ret == 0) {
                ret = frame_caniot2can(frame, &msg->msg);
                if (ret != 0) {
                        return ret;
                }
                can_tx_msg_queue(msg);
        }
        return ret;
}

static const struct caniot_drivers_api drv = {
        .entropy = entropy,
        .get_time = get_time,

        .persistent_read = NULL,
        .persistent_write = NULL,
        .rom_read = NULL,

        .send = send,
        .recv = NULL,
        .set_filter = NULL,
        .set_mask = NULL,

        .pending_telemetry = NULL,
};

static const struct caniot_controller_config cfg = {
        .store_telemetry = 0,
};

static struct caniot_controller controller = {
        .name = "avr-can-tool Controller",
        .uid = 0x00000001,
        .telemetry_db = NULL,
        .driv = &drv,
        .cfg = &cfg
};

static bool is_caniot_frame(can_message *msg)
{
        return msg->ext == 0 && msg->rtr == 0;
}

int caniot_controller_process_frame(can_message *msg)
{
        if (msg == 0) {
                return -EINVAL;
        }

        /* not processable as a CANIOT frame */
        if (is_caniot_frame(msg) == false) {
                return -CANIOT_ENPROC;
        }

        struct caniot_frame frame;
        int ret;

        ret = frame_can2caniot(msg, &frame);
        if (ret != 0) {
                return ret;
        }

        return caniot_controller_handle_rx_frame(&controller, &frame);
}

#endif /* defined(CONFIG_CANIOT_LIB) */