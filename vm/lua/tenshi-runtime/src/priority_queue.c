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

#include "inc/priority_queue.h"

struct _priority_queue {
  void *(*alloc_func)(void*, size_t);
  size_t capacity;
  size_t count;
  struct {
    int priority;
    void *obj;
  } *items;
};

static inline int parent(int nodeidx) {
  return (nodeidx - 1) / 2;
}

static inline int child(int nodeidx) {
  return 2 * nodeidx + 1;
}

#define DEFAULT_INITIAL_SIZE 8

priority_queue_t priority_queue_create(
  void *(*alloc_func)(void*, size_t), size_t initialsize) {
  if (initialsize == 0) {
    initialsize = DEFAULT_INITIAL_SIZE;
  }

  priority_queue_t ret = alloc_func(NULL, sizeof(struct _priority_queue));
  if (!ret) return NULL;

  ret->items = alloc_func(NULL, sizeof(ret->items[0]) * initialsize);
  if (!ret->items) return NULL;

  ret->alloc_func = alloc_func;
  ret->capacity = initialsize;
  ret->count = 0;

  return ret;
}

void priority_queue_free(priority_queue_t queue,
  void (*free_func)(void*)) {
  if (!queue) return;

  free_func(queue->items);
  free_func(queue);
}

int priority_queue_insert(priority_queue_t queue, void *obj, int pri) {
  size_t insert_pos = queue->count;

  // If we have exceeded the capacity, double it
  if (queue->count + 1 > queue->capacity) {
    queue->items = queue->alloc_func(queue->items,
      sizeof(queue->items[0]) * queue->capacity * 2);
    if (!queue->items) return -1;
    queue->capacity = queue->capacity * 2;
  }

  while (insert_pos > 0 && pri < queue->items[parent(insert_pos)].priority) {
    // Bubble up the item (or more precisely the hole)
    queue->items[insert_pos] = queue->items[parent(insert_pos)];
    insert_pos = parent(insert_pos);
  }

  // Insert the item in the hole
  queue->items[insert_pos].priority = pri;
  queue->items[insert_pos].obj = obj;
  queue->count++;

  return 0;
}

int priority_queue_peek_pri(priority_queue_t queue) {
  if (queue->count == 0) return -1;

  return queue->items[0].priority;
}

void *priority_queue_peek_obj(priority_queue_t queue) {
  if (queue->count == 0) return NULL;

  return queue->items[0].obj;
}

void *priority_queue_get_obj(priority_queue_t queue) {
  if (queue->count == 0) return NULL;

  void *retobj = queue->items[0].obj;
  queue->count--;
  // We have decreased count, but since we didn't clear the entry the former
  // last entry is still at queue->items[queue->count] ("x" in the Shewchuk
  // notes).

  // Where we are going to move the last item to ("x" in the Shewchuk notes).
  size_t insert_pos = 0;
  // The priority of the "x" item.
  int xpriority = queue->items[queue->count].priority;

  while (insert_pos < queue->count) {
    int child1pri = -1;
    int child2pri = -1;
    int child1exists = child(insert_pos) + 0 < queue->count;
    int child2exists = child(insert_pos) + 1 < queue->count;
    // We need to make sure that we don't ever read past the end of items.
    if (child1exists) child1pri = queue->items[child(insert_pos) + 0].priority;
    if (child2exists) child2pri = queue->items[child(insert_pos) + 1].priority;

    // If there is a child with a smaller priority...
    if ((child1exists && child1pri < xpriority) ||
        (child2exists && child2pri < xpriority)) {
      // We need to swap with the smallest child. If either 1 or 2 exists but
      // not both, it will always be 1 (the lower/left one).
      size_t smallestchild = child(insert_pos) + 0;
      if (child2exists && child2pri < child1pri) {
        smallestchild++;
      }

      // Bubble the hole down
      queue->items[insert_pos] = queue->items[smallestchild];
      insert_pos = smallestchild;
    } else {
      // There are no children or the children are greater
      break;
    }
  }

  // Insert the "x" item into the insert position
  queue->items[insert_pos] = queue->items[queue->count];

  return retobj;
}
