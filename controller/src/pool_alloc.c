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

#include "inc/pool_alloc.h"

#include <stdint.h>

struct _pool_alloc {
    // 1 bit is free, 0 bit is allocated
    // LSB is earlier block, MSB is later block
    uint32_t alloc_state;
    // Rounded up and including block header
    uint32_t block_size;
};

typedef struct _pool_block_hdr {
    struct _pool_alloc *pool;
    uint32_t idx;
} pool_block_hdr;

static size_t _pool_get_adjusted_block_size(size_t blocksize) {
    // Round up blocksize
    blocksize = (blocksize + 3) & (~3);

    // Add the block header size
    blocksize += sizeof(pool_block_hdr);

    return blocksize;
}

static pool_block_hdr *_pool_get_block_addr(pool_alloc_t pool, size_t idx) {
    return (pool_block_hdr *)(((uint8_t *)(pool)) +
        sizeof(struct _pool_alloc) + pool->block_size * idx);
}

// Returns the total size that this memory pool will require (in bytes).
size_t pool_alloc_get_size(size_t blocksize, size_t blockcnt) {
    // Check blockcnt
    if (blockcnt > 32) return 0;

    blocksize = _pool_get_adjusted_block_size(blocksize);

    return blocksize * blockcnt + sizeof(struct _pool_alloc);
}

// Allocate a new memory pool. Returns null on failure. The memory will be
// allocated using the passed-in allocator function. No cleanup is necessary
// to deallocate the pool.
pool_alloc_t pool_alloc_create(size_t blocksize, size_t blockcnt,
  void *(*alloc_func)(size_t)) {
    size_t size = pool_alloc_get_size(blocksize, blockcnt);

    if (!size) return NULL;

    pool_alloc_t pool = alloc_func(size);
    if (!pool) return NULL;

    pool->block_size = _pool_get_adjusted_block_size(blocksize);

    // This bit twiddling hack is used to clear all bits from bit index
    // blockcnt to 31 (inclusive). The end result is that there will only
    // be blockcnt 1 bits left.
    if (blockcnt == 32)
        pool->alloc_state = 0xFFFFFFFF;
    else
        pool->alloc_state = 0xFFFFFFFF & ((1 << blockcnt) - 1);

    // Mark all the headers to point back to the pool.
    for (int i = 0; i < blockcnt; i++) {
        pool_block_hdr *blkhdr = _pool_get_block_addr(pool, i);
        blkhdr->pool = pool;
        blkhdr->idx = i;
    }

    return pool;
}

// Allocates a block from the memory pool. Returns null on failure.
void *pool_alloc_block(pool_alloc_t pool) {
    if (!pool) return NULL;

    // No free blocks
    if (!pool->alloc_state) return NULL;

    // Should assemble into rbit, clz
    int idx = __builtin_ctz(pool->alloc_state);

    // Mark as allocated
    pool->alloc_state &= ~(1 << idx);

    // Get block address
    pool_block_hdr *blkhdr = _pool_get_block_addr(pool, idx);

    return ((uint8_t *)(blkhdr)) + sizeof(pool_block_hdr);
}

// Frees the passed-in block. Freeing null is not an error. Double free is
// undefined.
void pool_alloc_free(void *block) {
    if (!block) return;

    pool_block_hdr *blkhdr = (pool_block_hdr *)(block - sizeof(pool_block_hdr));
    pool_alloc_t pool = blkhdr->pool;
    pool->alloc_state |= (1 << blkhdr->idx);
}
