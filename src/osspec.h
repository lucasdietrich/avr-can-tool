#ifndef _OSSPEC_H
#define _OSSPEC_H

void entropy(uint8_t *buf, size_t len);

void get_time(uint32_t *sec, uint16_t *usec);

#endif /* _OSSPEC_H */
