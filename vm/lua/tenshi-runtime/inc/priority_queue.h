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

#ifndef INC_PRIORITY_QUEUE_H_
#define INC_PRIORITY_QUEUE_H_

#include <string.h>

// This module implements a priority queue using a binary heap stored in an
// array.

typedef struct _priority_queue *priority_queue_t;

// Allocate a new priority queue with the given default size. The priority
// queue will grow if too many items are added. The passed-in allocator
// should be a function that behaves like realloc.
extern priority_queue_t priority_queue_create(
  void *(*alloc_func)(void*, size_t), size_t initialsize);

// Free the priority queue.
extern void priority_queue_free(priority_queue_t queue,
  void (*free_func)(void*));

// Insert an object into the given queue with the given priority (lower numeric
// values are higher priority). Returns 0 on success.
extern int priority_queue_insert(priority_queue_t queue, void *obj, int pri);

// Peek at the item with highest priority and returs the priority. Returns -1
// if there are no items in the queue.
extern int priority_queue_peek_pri(priority_queue_t queue);

// Peek at the item with highest priority and returns the object. Returns NULL
// if there are no items in the queue.
extern void *priority_queue_peek_obj(priority_queue_t queue);

// Remove the item with the highest priority and returns the object. Returns
// NULL if there are no items in the queue.
extern void *priority_queue_get_obj(priority_queue_t queue);

#endif  // INC_PRIORITY_QUEUE_H_
