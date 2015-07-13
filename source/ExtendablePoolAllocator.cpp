// Copyright (C) 2015 ARM Limited. All rights reserved.

#include "mbed-util/ExtendablePoolAllocator.h"
#include "mbed-util/PoolAllocator.h"
#include "mbed-util/CriticalSectionLock.h"
#include "mbed-alloc/ualloc.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <new>

namespace mbed {
namespace util {

ExtendablePoolAllocator::ExtendablePoolAllocator(size_t elements, size_t new_pool_elements, size_t element_size, UAllocTraits_t alloc_traits, unsigned alignment):
    _elements(elements), _new_pool_elements(new_pool_elements), _alloc_traits(alloc_traits), _alignment(alignment) {
    _element_size = PoolAllocator::align_up(element_size, alignment);
}

bool ExtendablePoolAllocator::init() {
    _head = create_new_pool(_elements, NULL);
    return _head != NULL;
}

ExtendablePoolAllocator::~ExtendablePoolAllocator() {
    size_t aligned_ptr_size = PoolAllocator::align_up(sizeof(PoolAllocator*), _alignment);

    PoolAllocator *crt = _head, *prev;
    while (crt) {
        prev = get_ptr_to_prev(crt);
        crt->~PoolAllocator();
        mbed_ufree((void*)((uint32_t)crt - aligned_ptr_size));
        crt = prev;
    }
}

void* ExtendablePoolAllocator::alloc() {
    // Try the current pool first
    if (NULL == _head)
        return NULL;
    void *blk = _head->alloc();
    if (blk != NULL)
        return blk;

    // Try all the other pools
    PoolAllocator *prev_head = _head;
    PoolAllocator *crt = get_ptr_to_prev(prev_head);
    while (crt != NULL) {
        if ((blk = crt->alloc()) != NULL) {
            return blk;
        }
        crt = get_ptr_to_prev(crt);
    }
    // If someone else allocated a new pool meanwhile, use it
    if (prev_head != _head) {
        if ((blk = _head->alloc()) != NULL) {
            return blk;
        }
    }

    // Not enough space, need to create another pool
    prev_head = _head;
    {
        CriticalSectionLock lock; // execute with interrupts disabled
        if (_head != prev_head) { // if someone else already allocated a new pool, use it
            if ((blk = _head->alloc()) != NULL) {
                return blk;
            }
        }
        // Create a new pool and link it in the list of pools
        if ((crt = create_new_pool(_new_pool_elements, _head)) != NULL) {
            _head = crt;
            return crt->alloc();
        }
    }
    return NULL;
}

void *ExtendablePoolAllocator::calloc() {
    uint32_t *blk = (uint32_t*)alloc();

    if (blk == NULL)
        return NULL;
    for (unsigned i = 0; i < _element_size / 4; i ++, blk ++)
        *blk = 0;
    return blk;
}

void ExtendablePoolAllocator::free(void *p) {
    PoolAllocator *crt = _head;

    // Delegate freeing to the pool that owns the pointer
    while (crt != NULL) {
        if (crt->owns(p)) {
            crt->free(p);
            return;
        }
        crt = get_ptr_to_prev(crt);
    }
}

unsigned ExtendablePoolAllocator::get_num_pools() const {
    PoolAllocator *crt = _head;
    unsigned cnt = 0;

    while (crt != NULL) {
        cnt ++;
        crt = get_ptr_to_prev(crt);
    }
    return cnt;
}

PoolAllocator* ExtendablePoolAllocator::create_new_pool(size_t elements, PoolAllocator *prev) const {
    // Create a pool instance + the actual pool space + a link to the previous pool allocator in the chain in a contigous memory area.
    // Layout: pointer to previous pool | alignment(if any) | pool instance | alignment (if any) | pool storage area
    // Returns a pointer to the new pool instance
    size_t aligned_ptr_size = PoolAllocator::align_up(sizeof(PoolAllocator*), _alignment);
    size_t aligned_pool_size = PoolAllocator::align_up(sizeof(PoolAllocator), _alignment);
    size_t memsize = aligned_ptr_size + aligned_pool_size + PoolAllocator::get_pool_size(elements, _element_size, _alignment);
    void *temp = mbed_ualloc(memsize, _alloc_traits);
    if (temp == NULL)
        return NULL;
    void *pinst = (PoolAllocator*)((uint32_t)temp + aligned_ptr_size);
    void *pstorage = (void*)((uint32_t)temp + aligned_ptr_size + aligned_pool_size);
    *((PoolAllocator**)temp) = prev;
    return new(pinst) PoolAllocator(pstorage, _elements, _element_size, _alignment);
}

PoolAllocator* ExtendablePoolAllocator::get_ptr_to_prev(PoolAllocator *p) const {
    size_t aligned_ptr_size = PoolAllocator::align_up(sizeof(PoolAllocator*), _alignment);
    return *(PoolAllocator**)((uint32_t)p - aligned_ptr_size);
}

} // namespace util
} // namespace mbed

