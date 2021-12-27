#include <avrtos/kernel.h>

static K_PRNG_DEFINE_DEFAULT(prng);
static K_MUTEX_DEFINE(prng_mutex);

void entropy(uint8_t *buf, size_t len)
{
        k_mutex_lock(&prng_mutex, K_FOREVER);
        k_prng_get_buffer(&prng, buf, len);
        k_mutex_unlock(&prng_mutex);
}

void get_time(uint32_t *sec, uint16_t *msec)
{
        struct timespec ts;

        k_timespec_get(&ts);

        *sec = ts.tv_sec;

	if (msec != NULL) {
        	*msec = ts.tv_msec;
	}
}
