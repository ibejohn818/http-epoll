
#ifndef POOLS_H_
#define POOLS_H_

#include <stdbool.h>

#define MEM_POOL_ITEMS 512
#define MEM_POOL_ELEMENT (8 * 1024)

typedef struct {
  bool pooled;
  bool in_use;
  char memory[MEM_POOL_ELEMENT];
} memory_pool_item_t;


static memory_pool_item_t mem_pool[MEM_POOL_ITEMS]; 

void *memory_pool_get(void);
void memory_pool_release(void *p);

#endif
