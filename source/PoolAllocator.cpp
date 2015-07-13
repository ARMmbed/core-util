// Copyright (C) 2015 ARM Limited. All rights reserved.

#include "mbed-util/PoolAllocator.h"
#include "cmsis-core/core_generic.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace mbed {
namespace util {

PoolAllocator::PoolAllocator(void *start, size_t elements, size_t element_size, size_t alignment):
    _start(start), _element_size(element_size) {
    _element_size = (element_size + alignment - 1) & ~(alignment - 1);
    _end = (void*)((uint8_t*)start + _element_size * elements);
    _init();
}

void* PoolAllocator::alloc() {
    void **free;

    do {
        if((free = (void**)__LDREXW((uint32_t*)&_free_block)) == NULL) {
            __CLREX();
            break;
        }
    } while(__STREXW((uint32_t)*free, (uint32_t*)&_free_block));
    return (void*)free;
}

void PoolAllocator::free(void* p) {
    if(owns(p)) {
        do {
            *((void**)p) = (void*)__LDREXW((uint32_t*)&_free_block);
        } while(__STREXW((uint32_t)p, (uint32_t*)&_free_block));
    }
}

bool PoolAllocator::owns(void *p) const {
    return (p >= _start) && (p < _end);
}

size_t PoolAllocator::get_pool_size(size_t elements, size_t element_size, unsigned alignment) {
    element_size = (element_size + alignment - 1) & ~(alignment - 1);
    return element_size * elements;
}

void* PoolAllocator::calloc() {
    uint32_t *blk = (uint32_t*)alloc();

    if (NULL == blk)
        return NULL;
    for(unsigned i = 0; i < _element_size / 4; i ++, blk ++)
        *blk = 0;
    return blk;
}

void PoolAllocator::_init() {
    _free_block = _start;

    // Link all free blocks using offsets.
    void* next;
    void* blk = _free_block;

    while(true) {
        next = ((uint8_t *) blk) + _element_size;
        if(next > _end)
            break;
        *((void **)blk) = next;
        blk = next;
    }
    // count back one block and mark it as the last one
    blk = ((uint8_t *) blk) - _element_size;
    *((void **)blk) = 0;
}

} // namespace util
} // namespace mbed

