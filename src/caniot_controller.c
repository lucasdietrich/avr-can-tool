#include "caniot_controller.h"

#include <caniot.h>
#include <controller.h>

#include <avrtos/kernel.h>

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

static struct caniot_controller controller = {
        .name = "avr-can-tool Controller",
        .uid = 0x00000001,
        .driv = &drv
};

int process_caniot_frame(can_message *msg)
{
        struct caniot_frame frame;
        int ret;

        ret = frame_can2caniot(msg, &frame);
        if (ret != 0) {
                return ret;
        }

        return caniot_controller_handle_rx_frame(&controller, &frame);
}

struct k_signal query_sig;

static int query_callback(union deviceid did,
                          struct caniot_frame *resp)
{
        k_signal_raise(&query_sig, resp == NULL);

        return 0;
}

int request_telemetry(union deviceid did, uint8_t ep, k_timeout_t timeout)
{
        int ret;

        int32_t tm = K_TIMEOUT_EQ(timeout, K_FOREVER) ?
                ((int32_t)-1) : (int32_t)timeout.value;

        ret = caniot_request_telemetry(&controller, did, ep, query_callback, tm);
        if (ret != 0 && k_poll_signal(&query_sig, K_FOREVER) == 0) {
                K_SIGNAL_SET_UNREADY(&query_sig);
                printf("%hhx", query_sig.signal);
        }

        return ret;
}

#endif /* defined(CONFIG_CANIOT_LIB) */