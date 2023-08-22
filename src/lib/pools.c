
#include "http-epoll/pools.h"
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>



static pthread_mutex_t memory_pool_lock;

void *memory_pool_get(void) {

  pthread_mutex_lock(&memory_pool_lock);

  for(uint16_t i =0; i < MEM_POOL_ITEMS; i++) {

    if(mem_pool[i].is_use == false) {
      fprintf(stderr, "Mem idx: %u \n", i);
      mem_pool[i].is_use = true;

      void *item = &(mem_pool[i].memory);

      pthread_mutex_unlock(&memory_pool_lock);

      return item;
    }
  }

  pthread_mutex_unlock(&memory_pool_lock);
  return NULL;
}

void memory_pool_release(void *p) {
  for(uint16_t i =0; i < MEM_POOL_ITEMS; i++) {

    if (&(mem_pool[i].memory) == p) {

      memset(mem_pool[i].memory, 0, MEM_POOL_ELEMENT);
      mem_pool[i].is_use = false;
      return;

    }

  }
}
