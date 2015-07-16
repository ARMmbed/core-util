// Copyright (C) 2015 ARM Limited. All rights reserved.

#ifndef __MBED_UTIL_BINARY_HEAP_H__
#define __MBED_UTIL_BINARY_HEAP_H__

#include <stddef.h>
#include <stdint.h>
#include "mbed-util/CriticalSectionLock.h"
#include "mbed-util/Array.h"
#include "mbed-alloc/ualloc.h"
#include <stdio.h>

/** A reentrant binary heap class (https://en.wikipedia.org/wiki/Heap_(data_structure))
  * It uses the implicit representation: the heap's nodes are stored in an Array and accessed
  * by index. This kind of representation requires very little additional RAM.
  *
  * Becuase it uses an Array, it can grow automatically to accomodate more elements.
  *
  * The elements are sorted according to a user supplied comparison function, which can
  * make the implementation behave like a min-heap (the smallest element is always in the
  * root of the heap) or max-heap (the largest element is always in the root of the heap)
  */
namespace mbed {
namespace util {

template <typename T>
class BinaryHeap {
public:
    /** Heap's compare function
      * @param e1 first element to compare
      * @param e2 second element to compare
      * @returns true if e1 needs to come before e2 in the heap, false otherwise:
      *          - for a min heap, a simple implementation is 'return e1 <= e2'
      *          - for a max heap, a simple implementation is 'return e1 >= e2'
      */
    typedef bool (*cmp_fn_t)(const T& e1, const T& e2);

    /** Construct a new binary heap
      */
    BinaryHeap(): _array() {
    }

    /** Initialize the heap
      * @param initial_capacity initial capacity of the heap
      * @param grow_capacity number of elements to add when the heap's capacity is exceeded
      * @param alloc_traits allocator traits (for mbed_ualloc)
      * @param cmp_fn_t pointer to the heap's comparison function
      * @param alignment alignment of each element in the array
      * @returns true if the initialization succeeded, false otherwise
      */
    bool init(size_t initial_capacity, size_t grow_capacity, UAllocTraits_t alloc_traits, cmp_fn_t comp_func, unsigned alignment = MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN) {
        _comp_func = comp_func;
        _elements = 0;
        return _array.init(initial_capacity, grow_capacity, alloc_traits, alignment);
    }

    /** Inserts an element in the heap
      * @param p the element to insert
      * @returns true for success, false for failure (out of memory)
      */
    bool insert(const T& p) {
        CriticalSectionLock lock;
        if (!_array.push_back(p))
            return false;
        if (++_elements > 1) {
            _propagate_up(_elements - 1);
        }
        return true;
    }

    /** Returns a copy of the element in the root of the heap
      * @returns copy of the root
      */
    T get_root() const {
        if (_elements == 0) {
            MBED_UTIL_RUNTIME_ERROR("get_root() called on an empty BinaryHeap");
        }
        return _array[0];
    }

    /** Removes the element at the root of the heap, possibly re-shaping the heap
      * to keep it consistent
      */
    void remove_root() {
        if (_elements == 0)
            return;
        {
            CriticalSectionLock lock;
            _swap(0, --_elements); // element 0 will be destroyed by 'pop_back()' below
            _array.pop_back();
            if (_elements > 1) {
                _propagate_down(0);
            }
        }
    }

    /** Checks if the heap is empty
      * @returns true if the heap is empty, false otherwise
      */
    bool is_empty() const {
        return _elements == 0;
    }

    /** Remove an element from the heap. The element is searched in the heap by value using
      * the equality operator (==), then removed. If multiple elements with the same value
      * as 'e' are found, only the first one is removed.
      * @returns true if the element was found and removed, false otherwise.
      */
    bool remove(const T& e) {
        if (_elements == 0)
            return false;
        {
            CriticalSectionLock lock;
            unsigned i;
            for (i = 0; i < _elements; i ++) {
                if (e == _array[i])
                    break;
            }
            if (i == _elements)
                return false;
            _swap(i, --_elements); // element i will be destroyed by 'pop_back()' below
            _array.pop_back();
            if (_elements > 1)
                _propagate_down(i);
            return true;
        }
    }

    /** Check the heap's consistency by applying the user supplied comparison function to its nodes
      * @returns true if the heap is consistent, false otherwise
      */
    bool is_consistent(unsigned node = 0) const {
        if (node >= _elements)
            return true;
        unsigned left = _left(node), right = _right(node);
        if ((left < _elements) && !_comp_func(_array[node], _array[left]))
            return false;
        if ((right < _elements) && !_comp_func(_array[node], _array[right]))
            return false;
       return is_consistent(left) && is_consistent(right);
    }

    /** Returns the number of elements in the heap
      * @returns number of elements in the heap
      */
    size_t get_num_elements() const {
        return _elements;
    }

private:
    unsigned _left(unsigned i) const {
        return 2 * i + 1;
    }

    unsigned _right(unsigned i) const {
        return 2 * i + 2;
    }

    unsigned _parent(unsigned i) const {
        return (i - 1) / 2;
    }

    void _propagate_up(unsigned node) {
        // This is called when a node is added in the last position in the heap
        // We might need to move the node up towards the parent until the heap property
        // is satisfied
        unsigned parent = _parent(node);
        // Move the node up until it satisfies the heap property
        while ((node > 0) && _comp_func(_array[node], _array[parent])) {
            _swap(node, parent);
            node = parent;
            parent = _parent(node);
        }
    }

    void _propagate_down(unsigned node) {
        // This is called when an existing node is removed
        // When that happens, it is replaced with the node at the last position in the heap
        // Since that might make the heap inconsistent, we need to move the node down if its
        // value does not respect the comparison function when compared with its left and
        // right children. 
        while (true) {
            unsigned left = _left(node), right = _right(node), temp;
            bool change_left = (left < _elements) && !_comp_func(_array[node], _array[left]);
            bool change_right = (right < _elements) && !_comp_func(_array[node], _array[right]);
            if (change_left && change_right) {
                // If both left/right should change, use the comparison function to figure out
                // which way to go
                temp = _comp_func(_array[left], _array[right]) ? left : right;
            } else if (change_left) {
                temp = left;
            } else if (change_right) {
                temp = right;
            } else {
                break;
            }
            _swap(node, temp);
            node = temp;
        }
    }

    void _swap(unsigned pos1, unsigned pos2) {
        if (pos1 != pos2) {
            T temp = _array[pos1];
            _array[pos1] = _array[pos2];
            _array[pos2] = temp;
        }
    }

    Array<T> _array;
    cmp_fn_t _comp_func;
    volatile unsigned _elements;
};

} // namespace util
} // namespace mbed

#endif // #ifndef __MBED_UTIL_BINARY_HEAP_H__
