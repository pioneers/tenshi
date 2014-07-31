#include <ndl3.h>
#include <stdint.h> /* uint8_t, uint16_t, uint32_t. */
#include <string.h> /* memcpy */
#include <assert.h> /* assert */
#include <stdlib.h> /* malloc, free, size_t */

/**
 * Basically, works like this:
 * port is opened on host.
 * port is opened on target.
 * host has new L3 packet pushed to port.
 * host outputs START L2 packet, with size information.
 * If target receives START packet, it allocates room for L3 packet.
 * host begins outputting L2 packets, moving down the L3 packet.
 * If target started packet, those are collected.
 * If no ack is received before NDL3_ACK_TIMEOUT time has passed, host jumps
 * back to the last ACKed offset. If that offset is 0, a START is sent again.
 * If it receives packet type BACK, it also jumps back to the specified offset.
 * Once all of the ACKs have been received by the host, the host sends END.
 * If target receives END, it sends FIN back and closes.
 * If host receives FIN, it closes.
 * Otherwise, host will time out and also close (this is not an error).
 */


/* Packet slot is open for new packet. */
#define PACKET_EMPTY   1

/* Data is being sent or received. */
#define PACKET_OPEN    2

/* END or FIN received. */
#define PACKET_CLOSING 4

/* Everything has been ACKed, continue sending ACKs, and wait for END. */
#define PACKET_POPPED  8

/* Received out of order packet. */
#define PACKET_BACK   16


#define START_PACKET 1
#define ACK_PACKET   2
#define BACK_PACKET  3
#define DATA_PACKET  4
#define END_PACKET   5
#define FIN_PACKET   6

/*
 * TODO(kzentner): Consider refactoring header into single struct, and using
 * unions.
 */

/* Used for END and FIN. */
typedef struct L2_end_packet {
  uint16_t checksum;
  NDL3_port port;
  uint8_t type;
  uint16_t number;
} NDL3_PACKED L2_end_packet;

/* Used for ACK and BACK. */
typedef struct L2_ack_packet {
  uint16_t checksum;
  NDL3_port port;
  uint8_t type;
  uint16_t number;
  NDL3_size offset;
} NDL3_PACKED L2_ack_packet;

/* Used for START and DATA. */
typedef struct L2_data_packet {
  uint16_t checksum;
  NDL3_port port;
  uint8_t type;
  uint16_t number;

  /* Used for holding total_size in start packet. */
  NDL3_size offset;

  uint16_t size;
  uint8_t bytes[];
} NDL3_PACKED L2_data_packet;

/* Used for holding status of incoming and outgoing packets. */
typedef struct semi_packet {
  uint8_t * data;
  NDL3_size last_offset;
  NDL3_size last_acked_offset;
  NDL3_size total_size;
  NDL3_time time_last_ack; /* milliseconds */
  uint16_t number;
  int state;
} semi_packet;

/* Used for recording state of port. */
typedef struct port {
  semi_packet in_pkts[NDL3_PACKETS_PER_PORT];
  semi_packet out_pkts[NDL3_PACKETS_PER_PORT];
  NDL3_port num;
  uint16_t last_packet_num;
  NDL3_options opt;
} port;

struct NDL3Net {
  port ports[NDL3_MAXPORTS];
  NDL3_error last_error;
  void * userdata;
  NDAlloc * alloc;
  NDFree * free;
  int time;
};

/* Find the offset of a field. This will be optimized out anyways. */
#define OFFSET(strct, field) ((size_t) &((strct *) 0)->field)

static uint16_t checksum_packet(L2_data_packet * packet,
                                NDL3_size actual_size) {
  /*
   * This is the Fletcher-16 checksum.
   * See http://en.wikipedia.org/wiki/Fletcher%27s_checksum
   */
  uint8_t sum0 = 0;
  uint8_t sum1 = 0;
  uint8_t * s = (uint8_t *) &packet->port;
  NDL3_size size = actual_size - OFFSET(L2_data_packet, port);
  for (int i = 0; i < size; i++) {
    sum0 = (sum0 + s[i]) & 0xff;
    sum1 = (sum1 + sum0) & 0xff;
  }
  uint8_t check0 = (0xff - ((sum0 + sum1) & 0xff));
  uint8_t check1 = (0xff - ((sum0 + check0) & 0xff));
  return check0 | (check1 << 8);
}

/*
 * Create a new NDL3Net object. Stored alloc_fn and free_fn, which will be
 * passed userdata in later calls.
 */
NDL3Net * NDL3_new(NDAlloc alloc_fn, NDFree free_fn, void * userdata) {
  if (alloc_fn == NULL) {
    alloc_fn = &ND_malloc;
  }
  if (free_fn == NULL) {
    free_fn = &ND_free;
  }
  NDL3Net * net = (NDL3Net *) alloc_fn(sizeof(NDL3Net), userdata);
  if (net == 0) {
    return 0;
  }
  for (int i = 0; i < NDL3_MAXPORTS; i++) {
    net->ports[i].num = 0;
  }
  net->userdata = userdata;
  net->alloc = alloc_fn;
  net->free = free_fn;
  return net;
}

/*
 * Find the index of port in the port array. Returns -1 on error.
 */
static int port_idx(NDL3Net * restrict net, NDL3_port port) {
  for (int i = 0; i < NDL3_MAXPORTS; i++) {
    if (net->ports[i].num == port) {
      return i;
    }
  }
  net->last_error = NDL3_ERROR_PORT_NOT_OPEN;
  return -1;
}

/*
 * Open a port. Required before anything can be sent or received on port.
 */
void NDL3_open(NDL3Net * restrict net, NDL3_port port) {
  if (port_idx(net, port) >= 0) {
    net->last_error = NDL3_ERROR_PORT_ALREADY_OPEN;
    return;
  }
  int i;
  if ((i = port_idx(net, 0)) >= 0) {
    net->ports[i].num = port;
    for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
      net->ports->in_pkts[j].state = PACKET_EMPTY;
      net->ports->out_pkts[j].state = PACKET_EMPTY;
    }
    return;
  }
  net->last_error = NDL3_ERROR_NO_PORTS_LEFT;
}

/*
 * Close a port. In progress packets will be freed, and the remote side will be
 * left hanging. Currently this will result on the packets on the remote side
 * being forever stuck partially-sent (until the remote port is also closed and
 * opened).
 */
void NDL3_close(NDL3Net * restrict net, NDL3_port port) {
  int i;
  if ((i = port_idx(net, port)) >= 0) {
    for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
      if (net->ports[i].in_pkts[j].state != PACKET_EMPTY) {
        void * data = (void *) net->ports[i].in_pkts[j].data;
        if (data != NULL) {
          net->free(data, net->userdata);
        }
        net->ports[i].in_pkts[j].state = PACKET_EMPTY;
      }
      if (net->ports[i].out_pkts[j].state != PACKET_EMPTY) {
        void * data = (void *) net->ports[i].out_pkts[j].data;
        if (data != NULL) {
          net->free(data, net->userdata);
        }
        net->ports[i].out_pkts[j].state = PACKET_EMPTY;
      }
    }
    return;
  }
  net->last_error = NDL3_ERROR_PORT_ALREADY_CLOSED;
}

/*
 * Set a port option. There are currently no supported options.
 */
void NDL3_setopt(NDL3Net * restrict net, NDL3_port port, NDL3_options opt) {
  if (port_idx(net, port) >= 0) {
    return;
  }
  net->last_error = NDL3_ERROR_PORT_NOT_OPEN;
}

/*
 * Send a packet (msg), of precisely size bytes, over port.
 * If port is not opened on this side, an error will occur.
 * If port is not open on the other side, the packet will be sent when the port
 * is opened.
 *
 * msg becomes owned by NDL3Net, and will be freed using the user supplied free
 * function when the packet has been sent or the port is closed.
 */
void  NDL3_send(NDL3Net * restrict net, NDL3_port port,
                void * msg, NDL3_size size) {
  int i;
  if ((i = port_idx(net, port)) >= 0) {
    for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
      if (net->ports[i].out_pkts[j].state == PACKET_EMPTY) {
        /* Race point. */
        net->ports[i].out_pkts[j].state = PACKET_OPEN;
        net->ports[i].out_pkts[j].data = msg;
        net->ports[i].out_pkts[j].total_size = size;
        net->ports[i].out_pkts[j].last_offset = 0;
        net->ports[i].out_pkts[j].last_acked_offset = 0;
        net->ports[i].out_pkts[j].time_last_ack = net->time;
        net->ports[i].out_pkts[j].number = 1;
        return;
      }
    }
    net->last_error = NDL3_ERROR_NO_PACKETS_LEFT;
    return;
  }
  net->last_error = NDL3_ERROR_PORT_NOT_OPEN;
}

void NDL3_recv(NDL3Net * restrict net, NDL3_port port,
               void ** msg, NDL3_size * size) {
  int i;
  if ((i = port_idx(net, port)) >= 0) {
    for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
      semi_packet * pkt = &net->ports[i].in_pkts[j];
      if (pkt->state & PACKET_EMPTY) {
        /* Only check active packets. */
        continue;
      }
      /* If we have all the data and it hasn't been popped. */
      if (pkt->last_offset == pkt->total_size &&
          !(pkt->state & PACKET_POPPED)) {
        *msg = pkt->data;
        if (size != NULL) {
          *size = pkt->total_size;
        }
        pkt->data = 0;

        /*
         * Empty the packet slot if the packet's been closed
         * (END has been sent).
         */
        if (pkt->state & PACKET_CLOSING) {
          pkt->state = PACKET_EMPTY;
        }
        return;
      }
    }
    net->last_error = NDL3_ERROR_NO_PACKETS;
    return;
  }
  net->last_error = NDL3_ERROR_PORT_NOT_OPEN;
}

/*
 * Pop an L2 ACK or BACK packet. Responsible for checking for enough space in
 * destination.
 * Checks PACKET_BACK flag.
 */
static void pop_ack(NDL3Net * restrict net,
                     NDL3_port port,
                     semi_packet * pkt,
                     L2_ack_packet * rdest,
                     NDL3_size max_size,
                     NDL3_size * actual_size) {
  if (max_size < sizeof(L2_end_packet)) {
    net->last_error = NDL3_ERROR_L2_PACKET_TOO_SMALL;
    return;
  }

  NDL3_size size = sizeof(L2_ack_packet);
  *actual_size = size;

  rdest->port = port;

  /*
   * If the PACKET_BACK flag is set, we've lost a DATA packet, so send a BACK
   * packet.
   */
  if (pkt->state & PACKET_BACK) {
    rdest->type = BACK_PACKET;
    pkt->state = pkt->state & ~PACKET_BACK;
  } else {
    rdest->type = ACK_PACKET;
  }

  rdest->offset = pkt->last_offset;
  rdest->number = pkt->number;
  rdest->checksum = checksum_packet((L2_data_packet *) rdest, size);

  pkt->last_acked_offset = pkt->last_offset;
  pkt->time_last_ack = net->time;
}

/*
 * Pop an L2 START packet. Responsible for checking for enough space in
 * destination.
 */
static void pop_start(NDL3Net * restrict net,
                     NDL3_port port,
                     semi_packet * pkt,
                     L2_data_packet * rdest,
                     NDL3_size max_size,
                     NDL3_size * actual_size) {
  NDL3_size size = sizeof(L2_data_packet) + pkt->total_size -
      pkt->last_offset;
  if (size > max_size) {
    size = max_size;
  }

  /* Must send at least one byte. */
  if (size <= sizeof(L2_data_packet)) {
    net->last_error = NDL3_ERROR_L2_PACKET_TOO_SMALL;
    return;
  }
  *actual_size = size;

  rdest->port = port;
  rdest->type = START_PACKET;

  /* This is kinda hacky, but results in simplest code. */
  rdest->offset = pkt->total_size;

  rdest->number = pkt->number;
  rdest->size = size - sizeof(L2_data_packet);
  memcpy((void *) (rdest->bytes), (void *) pkt->data, rdest->size);
  rdest->checksum = checksum_packet((L2_data_packet *) rdest, size);
  pkt->last_offset += rdest->size;
  return;
}

/*
 * Pop an L2 DATA packet. Responsible for checking for enough space in
 * destination.
 */
static void pop_data(NDL3Net * restrict net,
                     NDL3_port port,
                     semi_packet * pkt,
                     L2_data_packet * rdest,
                     NDL3_size max_size,
                     NDL3_size * actual_size) {
  NDL3_size size = sizeof(L2_data_packet) + pkt->total_size -
      pkt->last_offset;

  if (size > max_size) {
    size = max_size;
  }

  /* Must send at least one byte. */
  if (size <= sizeof(L2_data_packet)) {
    net->last_error = NDL3_ERROR_L2_PACKET_TOO_SMALL;
    return;
  }

  *actual_size = size;

  rdest->port = port;
  rdest->type = DATA_PACKET;
  rdest->offset = pkt->last_offset;
  rdest->size = size - sizeof(L2_data_packet);
  memcpy((void *) (rdest->bytes), (void *) (pkt->data + pkt->last_offset),
         rdest->size);
  rdest->checksum = checksum_packet(rdest, size);

  pkt->last_offset += rdest->size;
  return;
}

/*
 * Pop an L2 END or FIN packet. Responsible for checking for enough space in
 * destination.
 * code should be either END_PACKET or FIN_PACKET.
 */
static void pop_end(NDL3Net * restrict net,
                    uint8_t code,
                    NDL3_port port,
                    semi_packet * pkt,
                    L2_end_packet * rdest,
                    NDL3_size max_size,
                    NDL3_size * actual_size) {
  if (max_size < sizeof(L2_end_packet)) {
    net->last_error = NDL3_ERROR_L2_PACKET_TOO_SMALL;
    return;
  }

  *actual_size = sizeof(L2_end_packet);
  rdest->port = port;
  rdest->type = code;
  rdest->number = pkt->number;
  pkt->state |= PACKET_CLOSING;

  if (pkt->state & PACKET_POPPED) {
    pkt->state = PACKET_EMPTY;
  }

  rdest->checksum = checksum_packet((L2_data_packet *) rdest,
                                    sizeof(L2_end_packet));
}

/**
 * Get an L2 packet to send.
 * dest is the destination to fill, with at most max_size bytes.
 * If actual_size is not NULL, its target is set to the number of bytes
 * actually used.
 */
void NDL3_L2_pop(NDL3Net * restrict net,
                 void * dest,
                 NDL3_size max_size,
                 NDL3_size * actual_size) {
  /* Make actual_size optional. */
  NDL3_size fake_size;
  if (actual_size == NULL) {
    actual_size = &fake_size;
  }

  for (int i = 0; i < NDL3_MAXPORTS; i++) {
    /* Skip closed ports. */
    if (net->ports[i].num == 0) {
      continue;
    }

    NDL3_port port = net->ports[i].num;
    for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
      /* Service incoming packets. */
      semi_packet * pkt = &net->ports[i].in_pkts[j];
      if (pkt->state & PACKET_OPEN) {
        if (pkt->state & PACKET_CLOSING) {
          /* If we are closing (have received END), send FIN. */
          pop_end(net, FIN_PACKET, port, pkt, dest, max_size, actual_size);
          return;
        }

        /*
         * If it has been NDL3_ACK_TIMEIN since last ack,
         * or NDL3_ACK_SPACEIN bytes have been sent since the last ack,
         * or we've reached the end of the packet and haven't acked that.
         */

        if (pkt->time_last_ack - net->time > NDL3_ACK_TIMEIN ||
            pkt->last_offset - pkt->last_acked_offset > NDL3_ACK_SPACEIN ||
            (pkt->last_offset == pkt->total_size &&
             pkt->last_offset != pkt->last_acked_offset)) {
          /* Send ack. */
          pop_ack(net, net->ports[i].num, pkt, (L2_ack_packet *) dest,
                  max_size, actual_size);
          return;
        }
      }

      /* Service outcoming packets. */
      pkt = &net->ports[i].out_pkts[j];

      if (pkt->state & PACKET_OPEN) {
        /*
         * If we've timed out or spaced out, reset back to the last ACKed
         * offset.
         */
        if (pkt->time_last_ack - net->time > NDL3_ACK_TIMEOUT ||
            pkt->last_offset - pkt->last_acked_offset > NDL3_ACK_SPACEOUT) {
          pkt->last_offset = pkt->last_acked_offset;
        }

        /*
         * If we're at the beginning of a packet, send START. Note that this
         * can be effected by the reset above.
         */
        if (pkt->last_offset == 0) {
          pop_start(net, port, pkt, (L2_data_packet *) dest,
                    max_size, actual_size);
          return;
        }

        if (pkt->total_size == pkt->last_acked_offset) {
          /* Everything has been acked, send END. */
          pop_end(net, END_PACKET, port, pkt,
                  (L2_end_packet *) dest, max_size, actual_size);
          return;
        }

        /* None of the above was needed, send data. */
        pop_data(net, port, pkt, (L2_data_packet *) dest,
                 max_size, actual_size);
      }
    }
  }
}

static void NDL3_L2_push_in_pkt(NDL3Net * restrict net, semi_packet * pkt,
                         L2_data_packet * L2pkt) {
  /*
   * Called for packets that are being received. Does not handle START packets,
   * because those are somewhat more complicated and thus have their own
   * function. START packets, by definition, also do not have an allocated
   * semi_packet.
   */
  if (L2pkt->type == END_PACKET) {
    pkt->state |= PACKET_CLOSING;
    net->free(pkt->data, net->userdata);
  } else if (L2pkt->type == DATA_PACKET) {
    if (L2pkt->offset != pkt->last_offset) {
      /*
       * We received an out of order data packet, so set the PACKET_BACK flag.
       * This will cause us to send a BACK packet to request the dropped
       * packet(s).
       */

      pkt->state |= PACKET_BACK;
    } else {
      memcpy((void *) (pkt->data + L2pkt->offset),
             (void *) L2pkt->bytes, L2pkt->size);
      pkt->last_offset += L2pkt->size;
    }
  } else {
    assert(0 && "Should not be reachable.");
  }
}

static void NDL3_L2_push_out_pkt(NDL3Net * restrict net, semi_packet * pkt,
                         L2_data_packet * L2pkt) {
  /* Called for packets that are being sent. */
  if (L2pkt->type == ACK_PACKET) {
    pkt->last_acked_offset = L2pkt->offset;
    pkt->time_last_ack = net->time;
  } else if (L2pkt->type == BACK_PACKET) {
    pkt->last_offset = L2pkt->offset;
    pkt->last_acked_offset = L2pkt->offset;
    pkt->time_last_ack = net->time;
  } else if (L2pkt->type == FIN_PACKET) {
    pkt->state |= PACKET_CLOSING;
    net->free(pkt->data, net->userdata);
  } else {
    assert(0 && "Should not be reachable.");
  }
}

static void push_start(NDL3Net * restrict net, NDL3_port port,
                L2_data_packet * L2pkt) {
  /*
   * Handle receiving START packet. Only place where memory is currently
   * allocated by this system.
   */
  for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
    semi_packet * pkt = &net->ports[port].in_pkts[j];
    if (pkt->state == PACKET_EMPTY) {
      /* Race point. */
      pkt->state = PACKET_OPEN;
      uint8_t * data = (uint8_t *) net->alloc(L2pkt->offset,
                                              net->userdata);
      if (data == NULL) {
        net->last_error = NDL3_ERROR_NO_MEMORY_LEFT;
        pkt->state = PACKET_EMPTY;
        return;
      }
      pkt->data = data;
      pkt->last_offset = L2pkt->size;
      pkt->last_acked_offset = 0;
      pkt->total_size = L2pkt->offset;
      pkt->time_last_ack = net->time;
      pkt->number = L2pkt->number;

      memcpy((void *) pkt->data, (void *) L2pkt->bytes, L2pkt->size);
      return;
    }
  }
  net->last_error = NDL3_ERROR_NO_PACKETS_LEFT;
  return;
}

/*
 * Push a received L2 packet.
 * msg is the L2 packet. It must not be NULL.
 * The size bytes after msg must be readable, but size need to be the exact
 * size of the L2 packet.
 * msg is not freed by this function.
 */
void NDL3_L2_push(NDL3Net * restrict net, void * msg, NDL3_size size) {
  L2_data_packet * L2pkt = (L2_data_packet *) msg;
  NDL3_size actual_size = size;
  /* end_packet is smallest packet type. Check that reading the type field is
   * safe. */
  if (size < sizeof(L2_end_packet)) {
    net->last_error = NDL3_ERROR_L2_PACKET_TOO_SMALL;
    return;
  }
  if (L2pkt->type == START_PACKET ||
      L2pkt->type == DATA_PACKET) {
    /* Extra check in case it's not safe to read size field. */
    if (size < sizeof(L2_data_packet)) {
      net->last_error = NDL3_ERROR_L2_PACKET_CORRUPT;
      return;
    }

    actual_size = ((L2_data_packet *) L2pkt)->size + sizeof(L2_data_packet);
  } else if (L2pkt->type == END_PACKET ||
             L2pkt->type == FIN_PACKET) {
    actual_size = sizeof(L2_end_packet);
  } else if (L2pkt->type == ACK_PACKET ||
             L2pkt->type == BACK_PACKET) {
    actual_size = sizeof(L2_ack_packet);
  } else if (L2pkt->type == DATA_PACKET) {
    /* Extra check in case it's not safe to read size field. */
    if (size < sizeof(L2_data_packet)) {
      net->last_error = NDL3_ERROR_L2_PACKET_CORRUPT;
      return;
    }

    actual_size = L2pkt->size + sizeof(L2_data_packet);
  } else {
    /* Unknown type, must be corrupt. */
    net->last_error = NDL3_ERROR_L2_PACKET_CORRUPT;
    return;
  }

  if (actual_size > size) {
    net->last_error = NDL3_ERROR_L2_PACKET_CORRUPT;
    return;
  }

  if (checksum_packet(L2pkt, actual_size) != L2pkt->checksum) {
    net->last_error = NDL3_ERROR_L2_PACKET_CORRUPT;
    return;
  }

  /* At this point, we should have a valid packet. */

  int i;
  if ((i = port_idx(net, L2pkt->port)) >= 0) {
    if (L2pkt->type == START_PACKET) {
      push_start(net, i, (L2_data_packet *) L2pkt);
      return;
    }
    if (L2pkt->type == START_PACKET ||
        L2pkt->type == DATA_PACKET ||
        L2pkt->type == END_PACKET) {
        /* We received an in-bound packet type. */
      for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
        semi_packet * pkt = &net->ports[i].in_pkts[j];
        if (!(pkt->state & PACKET_EMPTY) && pkt->number == L2pkt->number) {
          NDL3_L2_push_in_pkt(net, pkt, L2pkt);
          return;
        }
      }
    } else if (L2pkt->type == ACK_PACKET ||
               L2pkt->type == BACK_PACKET ||
               L2pkt->type == FIN_PACKET) {
      /* We have received an out-bound packet type. */
      for (int j = 0; j < NDL3_PACKETS_PER_PORT; j++) {
        semi_packet * pkt = &net->ports[i].out_pkts[j];
        if (!(pkt->state & PACKET_EMPTY) && pkt->number == L2pkt->number) {
          NDL3_L2_push_out_pkt(net, pkt, L2pkt);
          return;
        }
      }
    } else {
      assert(0 && "Should not be reachable.");
    }
  }

  net->last_error = NDL3_ERROR_L2_PACKET_IGNORED;
}

/*
 * Get the last error, and unsets it.
 */
NDL3_error NDL3_pop_error(NDL3Net * restrict net) {
  NDL3_error error = net->last_error;
  net->last_error = 0;
  return error;
}

/*
 * Record that some time has ellapsed. Needed for timeouts to be triggered.
 * Note that if this is not called, ACK's and spaceouts will still occur.
 */
void NDL3_elapse_time(NDL3Net * restrict net, NDL3_time time) {
  net->time += time;
}

/*
 * A wrapper around stdlib malloc. Provided for convenience.
 */
void * ND_malloc(NDL3_size size, void * userdata) {
  return malloc(size);
}

/*
 * A wrapper around stdlib free. Provided for convenience.
 */
void ND_free(void * to_free, void * userdata) {
  free(to_free);
}
