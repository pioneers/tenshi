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

#include "inc/priority_queue.h"

int queue_insert_one(void) {
  priority_queue_t queue = priority_queue_create(realloc, 0);
  if (!queue) return 1;

  if (priority_queue_insert(queue, (void*)12345, 1) != 0) return 1;

  if (priority_queue_peek_pri(queue) != 1) return 1;
  if (priority_queue_peek_obj(queue) != (void*)12345) return 1;

  priority_queue_free(queue, free);

  return 0;
}

int queue_remove_one(void) {
  priority_queue_t queue = priority_queue_create(realloc, 0);
  if (!queue) return 1;

  if (priority_queue_insert(queue, (void*)12345, 1) != 0) return 1;

  if (priority_queue_get_obj(queue) != (void*)12345) return 1;

  if (priority_queue_peek_pri(queue) != -1) return 1;

  priority_queue_free(queue, free);

  return 0;
}

int queue_insert_multiple_inorder(void) {
  // This function happens to exercise all of the remove bubbling code paths
  // (no children, 1 child, 2 children)

  priority_queue_t queue = priority_queue_create(realloc, 0);
  if (!queue) return 1;

  if (priority_queue_insert(queue, (void*)12345, 1) != 0) return 1;
  if (priority_queue_insert(queue, (void*)23456, 2) != 0) return 1;
  if (priority_queue_insert(queue, (void*)34567, 3) != 0) return 1;
  if (priority_queue_insert(queue, (void*)45678, 4) != 0) return 1;
  if (priority_queue_insert(queue, (void*)56789, 5) != 0) return 1;


  if (priority_queue_get_obj(queue) != (void*)12345) return 1;
  if (priority_queue_get_obj(queue) != (void*)23456) return 1;
  if (priority_queue_get_obj(queue) != (void*)34567) return 1;
  if (priority_queue_get_obj(queue) != (void*)45678) return 1;
  if (priority_queue_get_obj(queue) != (void*)56789) return 1;

  if (priority_queue_peek_pri(queue) != -1) return 1;

  priority_queue_free(queue, free);

  return 0;
}

int queue_insert_multiple_noorder(void) {
  // This function exercises the insert bubbling code paths

  priority_queue_t queue = priority_queue_create(realloc, 0);
  if (!queue) return 1;

  if (priority_queue_insert(queue, (void*)34567, 3) != 0) return 1;
  if (priority_queue_insert(queue, (void*)12345, 1) != 0) return 1;
  if (priority_queue_insert(queue, (void*)45678, 4) != 0) return 1;
  if (priority_queue_insert(queue, (void*)23456, 2) != 0) return 1;
  if (priority_queue_insert(queue, (void*)56789, 5) != 0) return 1;


  if (priority_queue_get_obj(queue) != (void*)12345) return 1;
  if (priority_queue_get_obj(queue) != (void*)23456) return 1;
  if (priority_queue_get_obj(queue) != (void*)34567) return 1;
  if (priority_queue_get_obj(queue) != (void*)45678) return 1;
  if (priority_queue_get_obj(queue) != (void*)56789) return 1;

  if (priority_queue_peek_pri(queue) != -1) return 1;

  priority_queue_free(queue, free);

  return 0;
}

int queue_insert_resize(void) {
  priority_queue_t queue = priority_queue_create(realloc, 1);
  if (!queue) return 1;

  if (priority_queue_insert(queue, (void*)34567, 3) != 0) return 1;
  if (priority_queue_insert(queue, (void*)12345, 1) != 0) return 1;
  if (priority_queue_insert(queue, (void*)45678, 4) != 0) return 1;
  if (priority_queue_insert(queue, (void*)23456, 2) != 0) return 1;
  if (priority_queue_insert(queue, (void*)56789, 5) != 0) return 1;


  if (priority_queue_get_obj(queue) != (void*)12345) return 1;
  if (priority_queue_get_obj(queue) != (void*)23456) return 1;
  if (priority_queue_get_obj(queue) != (void*)34567) return 1;
  if (priority_queue_get_obj(queue) != (void*)45678) return 1;
  if (priority_queue_get_obj(queue) != (void*)56789) return 1;

  if (priority_queue_peek_pri(queue) != -1) return 1;

  priority_queue_free(queue, free);

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

  run_test(queue_insert_one);
  run_test(queue_remove_one);

  run_test(queue_insert_multiple_inorder);
  run_test(queue_insert_multiple_noorder);

  run_test(queue_insert_resize);

  return failures;
}
