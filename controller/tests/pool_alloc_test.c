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

#include <stdio.h>
#include <stdlib.h>

#include "inc/pool_alloc.h"

int pool_size_calc(void) {
    // Note: This test requires 32-bit-ness.
    return pool_alloc_get_size(4, 1) != 20;
}

int pool_size_rounding(void) {
    // Note: This test requires 32-bit-ness.
    return pool_alloc_get_size(1, 1) != 20;
}

int pool_basic_alloc(void) {
    // Create a new pool
    pool_alloc_t pool = pool_alloc_create(1, 32, malloc);
    if (!pool) return 1;

    // Try allocating all 32 blocks
    for (int i = 0; i < 32; i++) {
        void *block = pool_alloc_block(pool);
        if (!block) return 1;
    }

    // Try allocating yet another block.
    void *block = pool_alloc_block(pool);
    if (block) return 1;

    // Cleanup
    free(pool);
    return 0;
}

int pool_basic_alloc_limited(void) {
    // Create a new pool
    pool_alloc_t pool = pool_alloc_create(1, 4, malloc);
    if (!pool) return 1;

    // Try allocating all 4 blocks
    for (int i = 0; i < 4; i++) {
        void *block = pool_alloc_block(pool);
        if (!block) return 1;
    }

    // Try allocating yet another block.
    void *block = pool_alloc_block(pool);
    if (block) return 1;

    // Cleanup
    free(pool);
    return 0;
}

int pool_alloc_free_alloc(void) {
    // Create a new pool
    pool_alloc_t pool = pool_alloc_create(1, 2, malloc);
    if (!pool) return 1;

    // Try allocating blocks
    void *block0 = pool_alloc_block(pool);
    if (!block0) return 1;
    void *block1 = pool_alloc_block(pool);
    if (!block1) return 1;

    // Free block0
    pool_alloc_free(block0);

    // Try allocating another block
    void *blockx = pool_alloc_block(pool);
    if (!blockx) return 1;

    // This alloc should fail
    void *blocky = pool_alloc_block(pool);
    if (blocky) return 1;

    // Cleanup
    free(pool);
    return 0;
}

#define run_test(X) {                                       \
  int ret = X();                                            \
  if (ret != 0) {                                           \
    failures++;                                             \
    fprintf(stderr, "Test %s FAILED!\n", #X);               \
  } else {                                                  \
    printf("Test %s passed.\n", #X);                        \
  }                                                         \
}
int main(int argc, char **argv) {
  int failures = 0;

  run_test(pool_size_calc);
  run_test(pool_size_rounding);
  run_test(pool_basic_alloc);
  run_test(pool_basic_alloc_limited);

  run_test(pool_alloc_free_alloc);

  return failures;
}
