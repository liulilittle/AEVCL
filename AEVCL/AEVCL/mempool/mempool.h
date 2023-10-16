#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

void* mempool_alloc(size_t size);
void* mempool_alloc(size_t size, size_t);
void mempool_free(void* ptr);

#endif