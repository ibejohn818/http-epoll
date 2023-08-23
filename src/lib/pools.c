
#include "http-epoll/pools.h"
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



static pthread_mutex_t memory_pool_lock;

void *memory_pool_get(void) {

  pthread_mutex_lock(&memory_pool_lock);

  for(uint16_t i =0; i < MEM_POOL_ITEMS; i++) {

    if(mem_pool[i].in_use == false) {
      fprintf(stderr, "Mem idx: %u \n", i);
      mem_pool[i].in_use = true;
      mem_pool[i].pooled = true;

      void *item = &(mem_pool[i].memory);

      pthread_mutex_unlock(&memory_pool_lock);

      return item;
    }
  }

  // if we reached this point, we didn't have
  // any pooled items available, allocate on the heap
  // memory_pool_item_t *i = (memory_pool_item_t *)malloc(sizeof(*i));

  pthread_mutex_unlock(&memory_pool_lock);

  return NULL;
}

void memory_pool_release(void *p) {

  /*
  // check if items is not from the pool
  if (((memory_pool_item_t *)p)->pooled == false) {
    memory_pool_item_t *i = (memory_pool_item_t *)p;
    free(i->memory);
    free(i);
  }
  */

  for(uint16_t i =0; i < MEM_POOL_ITEMS; i++) {

    if (&(mem_pool[i].memory) == p) {

      memset(mem_pool[i].memory, 0, MEM_POOL_ELEMENT);
      mem_pool[i].in_use = false;
      return;

    }

  }
}
