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

#ifndef INC_SMARTSENSOR_COBS_H_
#define INC_SMARTSENSOR_COBS_H_

#include <stdint.h>
// For size_t
#include <string.h>

// We don't hide the fields here so the user can just statically allocate this
// struct instead of having to use malloc.
typedef struct tag_cobs_decode_state {
  uint8_t current_block_len;
  uint8_t orig_block_len;
  uint8_t initial;
} cobs_decode_state;

// Encodes src into dst and returns the size of dst. Note that dst will have no
// more overhead than 1 byte per 254 bytes. src must not overlap dst.
extern size_t cobs_encode(uint8_t *dst, const uint8_t *src, size_t src_len);

// Decodes src into dst and returns the size of dst. src may overlap dst.
extern size_t cobs_decode(uint8_t *dst, const uint8_t *src, size_t src_len);

#define COBS_STREAM_END   (-1)
#define COBS_STREAM_DUMMY (-2)
// Decodes COBS data one byte at a time. Pass in COBS_STREAM_END to signal
// end of data. Function will return either a uint8_t in the low bits or will
// return COBS_STREAM_DUMMY or COBS_STREAM_END.
extern int cobs_decode_stream(cobs_decode_state *state, int c);

#endif  // INC_SMARTSENSOR_COBS_H_
