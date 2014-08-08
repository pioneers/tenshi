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

// This file will be magically invoked by malloc in the right places
// to lock various data structures.

#include <malloc.h>

#include "inc/FreeRTOS.h"
#include "inc/semphr.h"

static SemaphoreHandle_t heap_mutex;
// We need to not take the mutex before we've created the mutex. The mutex
// creation uses malloc. This works around that.
static has_init = 0;

void init_malloc_lock(void) {
  heap_mutex = xSemaphoreCreateRecursiveMutex();
  has_init = 1;
}

void __malloc_lock(struct _reent *REENT) {
  if (!has_init) return;
  xSemaphoreTakeRecursive(heap_mutex, portMAX_DELAY);
}

void __malloc_unlock(struct _reent *REENT) {
  if (!has_init) return;
  xSemaphoreGiveRecursive(heap_mutex);
}
