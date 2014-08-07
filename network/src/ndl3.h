#ifndef NDL3_H_

#define NDL3_H_
#include <stdint.h>

#ifdef __GNUC__
#define NDL3_PACKED __attribute__((packed))
#else
#define NDL3_PACKED
#endif

#define NDL3_MAXPORTS 4
#define NDL3_PACKETS_PER_PORT 4
#define NDL3_ACK_TIMEIN 100
#define NDL3_ACK_SPACEIN 100
#define NDL3_ACK_TIMEOUT 500
#define NDL3_ACK_SPACEOUT 500
#define NDL3_ACK_TIMEDIE 50000

#define NDL3_ERROR_L2_PACKET_CORRUPT   1
#define NDL3_ERROR_L2_PACKET_IGNORED   2
#define NDL3_ERROR_L2_PACKET_TOO_SMALL 3
#define NDL3_ERROR_NO_MEMORY_LEFT      4
#define NDL3_ERROR_NO_PACKETS          5
#define NDL3_ERROR_NO_PORTS_LEFT       6
#define NDL3_ERROR_NO_PACKETS_LEFT     7
#define NDL3_ERROR_PORT_ALREADY_CLOSED 8
#define NDL3_ERROR_PORT_ALREADY_OPEN   9
#define NDL3_ERROR_PORT_NOT_OPEN       10
#define NDL3_ERROR_BAD_MSG             11
#define NDL3_ERROR_PACKET_LOST         12
#define NDL3_ERROR_INCOMPLETE_SEND     13

struct NDL3Net;
typedef struct NDL3Net NDL3Net;

typedef uint8_t NDL3_port;

typedef uint32_t NDL3_error;

typedef uint32_t NDL3_options;

typedef uint32_t NDL3_time;

typedef uint32_t NDL3_size;

typedef void * NDAlloc(NDL3_size size, void * userdata);
typedef void NDFree(void * to_free, void * userdata);

extern NDAlloc ND_malloc;
extern NDFree ND_free;

NDL3Net * NDL3_new(NDAlloc * alloc_fn, NDFree * free_fn, void * userdata);

void NDL3_open(NDL3Net * restrict net, NDL3_port port);
void NDL3_close(NDL3Net * restrict net, NDL3_port port);

void NDL3_setopt(NDL3Net * restrict net, NDL3_port port, NDL3_options opt);

void NDL3_send(NDL3Net * restrict net, NDL3_port port, void *  msg,
               NDL3_size   size);
void NDL3_recv(NDL3Net * restrict net, NDL3_port port, void ** msg,
               NDL3_size * size);

void NDL3_L2_pop(NDL3Net * restrict net, void * dest, NDL3_size max_size,
                 NDL3_size * out_actual_size);
void NDL3_L2_push(NDL3Net * restrict net, void * msg, NDL3_size max_size);

/*
 * The time is not in specified units, but the timeouts above should be in the
 * same unit as the argument to this function.
 */
void NDL3_elapse_time(NDL3Net * restrict net, NDL3_time time);
NDL3_error NDL3_pop_error(NDL3Net * restrict net);

/*
 * Implements reliable, arbitrary (32 bit sized) packet transport.
 * Current limitations:
 *   - Does not distribute bandwidth evenly, packets / ports are favored
 *     haphazardly.
 *   - Does not implement packet ordering.
 *   - Connections do not timeout. If one side of a packet / port closes, the
 *     other side will be left hanging until it is explicitly closed as well.
 *   - Point-to-point only.
 *   - There's no way to list open ports.
 *   - There's no way to destroy the NDL3Net itself, although ports can be
 *     closed.
 * Intentional limitations:
 *   - Not thread safe. Users of this library should do their own blocking.
 */

/* TODO(kzentner): Fix non-intentional limitations above. */

#endif  // NDL3_H_
