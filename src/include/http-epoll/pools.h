
#ifndef POOLS_H_
#define POOLS_H_

#include <stdbool.h>

#define MEM_POOL_ITEMS 2048
#define MEM_POOL_ELEMENT 8192

typedef struct {
  bool is_use;
  char memory[MEM_POOL_ELEMENT];
} memory_pool_item_t;


static memory_pool_item_t mem_pool[MEM_POOL_ITEMS]; 

void *memory_pool_get(void);
void memory_pool_release(void *p);

#endif
