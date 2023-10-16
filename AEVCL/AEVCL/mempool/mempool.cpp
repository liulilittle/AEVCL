#include "mempool.h"
#include "nedmalloc.h"

void* mempool_alloc(size_t size, size_t align)
{
	const size_t SIMD_ALIGNMENT = 16;
	return align ? nedalloc::nedmemalign(align, size) : nedalloc::nedmemalign(SIMD_ALIGNMENT, size);
}

void* mempool_alloc(size_t size)
{
	return nedalloc::nedmalloc(size);
}

void mempool_free(void* ptr)
{
	nedalloc::nedfree(ptr);
}
