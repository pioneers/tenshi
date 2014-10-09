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

#ifndef INC_POOL_ALLOC_H_
#define INC_POOL_ALLOC_H_

#include <string.h>

// This module implements a memory pool allocator. Because of the optimizations
// used in this module, there are at most 32 memory blocks allowed. Each memory
// block is rounded up to a multiple of 4 bytes.

typedef struct _pool_alloc *pool_alloc_t;

// Returns the total size that this memory pool will require (in bytes).
// If the input is invalid (too many blocks) the return will be 0.
extern size_t pool_alloc_get_size(size_t blocksize, size_t blockcnt);

// Allocate a new memory pool. Returns null on failure. The memory will be
// allocated using the passed-in allocator function. No cleanup is necessary
// to deallocate the pool.
extern pool_alloc_t pool_alloc_create(size_t blocksize, size_t blockcnt,
  void *(*alloc_func)(size_t));

// Allocates a block from the memory pool. Returns null on failure.
extern void *pool_alloc_block(pool_alloc_t pool);

// Frees the passed-in block. Freeing null is not an error. Double free is
// undefined.
extern void pool_alloc_free(void *block);

#endif  // INC_POOL_ALLOC_H_
