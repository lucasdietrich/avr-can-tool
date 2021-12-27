#include "caniot_controller.h"

#include <caniot.h>
#include <controller.h>

#include <avrtos/kernel.h>

#include "osspec.h"
#include "can.h"

#if defined(CONFIG_CANIOT_LIB)

void controller_thread(void *ctx);

#define CANIOT_CONTROLLER_QUEUE_SIZE 2
char caniot_frame_q_buf[CANIOT_CONTROLLER_QUEUE_SIZE * sizeof(struct caniot_frame)];
K_MSGQ_DEFINE(caniot_frame_q, caniot_frame_q_buf,
	      sizeof(struct caniot_frame), CANIOT_CONTROLLER_QUEUE_SIZE);

K_THREAD_DEFINE(caniot_controller, controller_thread, 0x100, K_COOPERATIVE, NULL, 'C');

static int caniot2msg(can_message *msg, const struct caniot_frame *frame)
{
        if (!frame || !msg) {
                return -EINVAL;
        }

        memcpy(msg->buf, frame->buf, frame->len);
        msg->len = frame->len;
        msg->std = frame->id.raw;
        msg->rtr = 0;
        msg->isext = 0;

        return 0;
}

static int msg2caniot(struct caniot_frame *frame, const can_message *msg)
{
        if (!frame || !msg) {
                return -EINVAL;
        }

        if (msg->isext || msg->rtr) {
                return -EINVAL;
        }

        memcpy(frame->buf, msg->buf, msg->len);
        frame->len = msg->len;
        frame->id.raw = msg->std;

        return 0;
}

int queue_caniot_frame(const can_message *p_msg)
{
	struct caniot_frame frame;
	int ret;

	if (!p_msg) {
		return -EINVAL;
	}

	ret = msg2caniot(&frame, p_msg);
	if (ret) {
		return ret;
	}

	if(caniot_controller_is_target(&frame) == false) {
		return -EINVAL;
	}

	return k_msgq_put(&caniot_frame_q, &frame, K_NO_WAIT);
}

static int send(const struct caniot_frame *frame, uint32_t delay)
{
        ARG_UNUSED(delay);

	caniot_show_frame(frame);

        can_message_qi *msg;

        int8_t ret = can_msg_alloc(&msg, K_FOREVER);
        if (ret == 0) {
                ret = caniot2msg(&msg->msg, frame);
                if (ret != 0) {
                        return ret;
                }
                can_tx_msg_queue(msg);
        }
        return ret;
}

static int recv(struct caniot_frame *frame)
{
	return k_msgq_get(&caniot_frame_q, frame, K_NO_WAIT);
}

static const struct caniot_drivers_api drv = {
        .entropy = entropy,
        .get_time = get_time,

        .send = send,
        .recv = recv,
};

static struct caniot_controller controller = {
        .name = "avr-can-tool Controller",
        .uid = 0x00000001,
        .driv = &drv
};

void controller_thread(void *ctx)
{
	int ret;

	ret = caniot_controller_init(&controller);
	if (ret != 0) {
		printf_P(PSTR("Failed to initialize CANIOT controller: -%04x\n"), -ret);
		__fault(K_FAULT);
	}

	for(;;) {
		caniot_controller_process(&controller);

		k_sleep(K_MSEC(100));
	}
}

static int32_t timeout2signed(k_timeout_t timeout)
{
	if (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		return -1;
	} else {
		return (int32_t) timeout.value;
	}
}

static int cb(union deviceid did, struct caniot_frame *resp)
{
	if (resp == NULL) {
		printf_P(PSTR("Timeout\n"));
	} else {
		caniot_explain_frame(resp);
	}

	return 0;
}

int request_telemetry(union deviceid did, uint8_t ep, k_timeout_t timeout)
{
	K_SCHED_LOCK_CONTEXT{
		return caniot_request_telemetry(&controller, did, ep, cb, timeout2signed(timeout));
	}

	__builtin_unreachable();
}

#endif /* defined(CONFIG_CANIOT_LIB) */