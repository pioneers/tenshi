// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#include "inc/smartsensor/cobs.h"

// Functions in this file adapted from
// http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing

#define finish_block() {      \
  *block_len_loc = block_len; \
  block_len_loc = dst++;      \
  out_len++;                  \
  block_len = 0x01;           \
}
size_t cobs_encode(uint8_t *dst, const uint8_t *src, size_t src_len) {
  const uint8_t *end = src + src_len;
  uint8_t *block_len_loc = dst++;
  uint8_t block_len = 0x01;
  size_t out_len = 0;

  while (src < end) {
    if (*src == 0) {
      finish_block();
    } else {
      *dst++ = *src;
      block_len++;
      out_len++;
      if (block_len == 0xFF) {
        finish_block();
      }
    }
    src++;
  }
  finish_block();

  return out_len;
}

size_t cobs_decode(uint8_t *dst, const uint8_t *src, size_t src_len) {
  const uint8_t *end = src + src_len;
  size_t out_len = 0;

  while (src < end) {
    uint8_t code = *src++;
    for (uint8_t i = 1; i < code; i++) {
      *dst++ = *src++;
      out_len++;
    }
    if (code < 0xFF && src != end) {
      *dst++ = 0;
      out_len++;
    }
  }

  return out_len;
}

int cobs_decode_stream(cobs_decode_state *state, int c) {
  if (state->current_block_len == 0) {
    // The next byte will be a block start (or an end).
    if (c == COBS_STREAM_END) {
      // We're done, so we don't output the trailing zero.
      state->initial = 0;
      return COBS_STREAM_END;
    } else {
      // We have another block coming in. Our current block is done though, so
      // we have to output a zero.

      int ret;
      if (state->orig_block_len < 0xFF && state->initial) {
        ret = 0;
      } else {
        ret = COBS_STREAM_DUMMY;
      }

      state->orig_block_len = c;
      // We subtract 1 because the block lengths in COBS include the trailing
      // 0. current_block_len stores the number of ADDITIONAL characters in the
      // block.
      state->current_block_len = c - 1;
      // Used to prevent outputting an extra 0 at the very very beginning of a
      // stream.
      state->initial = 1;

      return ret;
    }
  } else {
    state->current_block_len--;
    return c;
  }
}
