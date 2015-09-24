/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CORE_UTIL_SHAREDPOINTER_H__
#define __CORE_UTIL_SHAREDPOINTER_H__

#include "core-util/core-util.h"
#include "mbed-alloc/ualloc.h"

#include <stdint.h>
#include <stddef.h>

#ifndef NDEBUG
#include <stdio.h>
#define CORE_UTIL_SHAREDPOINTER_DEBUG(...) { printf(__VA_ARGS__); }
#else
#define CORE_UTIL_SHAREDPOINTER_DEBUG(...) /* nothing */
#endif

namespace mbed {
namespace util {

/** Shared pointer class.
  *
  * Similar to std::shared_ptr in C++11.
  *
  * Usage: SharedPointer<class> POINTER(new class())
  *
  * When POINTER is passed around by value the copy constructor and
  * destructor counts the number of references to the original object.
  * If the counter reaches zero, delete is called on the object pointed to.
  *
  * To avoid loops, "weak" references should be used by calling the original
  * pointer directly through POINTER.get().
  */

template <class T>
class SharedPointer {
public:
    /** Create empty SharedPointer not pointing to anything.
      * Used for variable declaration.
      */
    SharedPointer(): pointer(NULL), counter(NULL) {
        CORE_UTIL_SHAREDPOINTER_DEBUG("SP: %p [%p: %p]\r\n", this, pointer, counter);
    }

    /** Create new SharedPointer
      * @param _pointer Pointer to take control over
      */
    SharedPointer(T* _pointer): pointer(_pointer) {
        CORE_UTIL_ASSERT(pointer);

        // allocate counter on the heap so it can be shared
        UAllocTraits_t traits = {0};
        counter = (uint32_t*) mbed_ualloc(sizeof(uint32_t), traits);

        // initialize counter to 1
        CORE_UTIL_ASSERT(counter);
        *counter = 1;

        CORE_UTIL_SHAREDPOINTER_DEBUG("SP: %p [%p: %p = %lu]\r\n", this, pointer, counter, *counter);
    }

    ~SharedPointer() {
        decrementCounter();
    }

    /** Copy constructor
      * Create new SharePointer from other SharedPointer by
      * copying pointer to original object and pointer to counter.
      * @param source object being copied from
      */
    SharedPointer(const SharedPointer& source): pointer(source.pointer), counter(source.counter) {
        // increment reference counter
        if (counter) {
            (*counter)++;
        }

        CORE_UTIL_SHAREDPOINTER_DEBUG("SP&: %p = %p [%p: %p = %lu]\r\n", this, &source, pointer, counter, *counter);
    }

    /** Assignment operator.
      * Cleanup previous reference and assign new pointer and counter.
      * @param source object being assigned
      */
    SharedPointer operator=(const SharedPointer& source) {
        if (this != &source) {
            // clean up by decrementing counter
            decrementCounter();

            // assign new values
            pointer = source.get();
            counter = source.getCounter();

            // increment new counter
            if (counter) {
                (*counter)++;
            }

            CORE_UTIL_SHAREDPOINTER_DEBUG("SP=: %p = %p [%p: %p = %lu]\r\n", this, &source, pointer, counter, *counter);
        }

        return *this;
    }

    /** Raw pointer accessor.
      * Get raw pointer to object pointed to.
      */
    T* get() const {
        return pointer;
    }

    /** Reference count accessor.
      * Return reference count.
      */
    uint32_t use_count() const {
        if (counter) {
            return *counter;
        } else {
            return 0;
        }
    }

    /** Dereference object operator.
      * Override to return the object pointed to.
      */
    T& operator*() const {
        CORE_UTIL_ASSERT(pointer);

        return *pointer;
    }

    /** Dereference object member operator.
      * Override to return return member in object pointed to.
      */
    T* operator->() const {
        CORE_UTIL_ASSERT(pointer);

        return pointer;
    }

    /** Boolean conversion operator.
      */
    operator bool() const {
        CORE_UTIL_SHAREDPOINTER_DEBUG("bool: %p\r\n", pointer);

        return (pointer != 0);
    }

private:
    /** Get pointer to reference counter.
      */
    uint32_t* getCounter() const {
        return counter;
    }

    /** Decrement reference counter.
      * If count reaches zero, free counter and delete object pointed to.
      */
    void decrementCounter() {
        if (counter) {
            if (*counter == 1) {
                mbed_ufree(counter);
                delete pointer;

                CORE_UTIL_SHAREDPOINTER_DEBUG("~SP: %p [%p: %p = 0]\r\n", this, pointer, counter);
            } else {
                (*counter)--;

                CORE_UTIL_SHAREDPOINTER_DEBUG("~SP: %p [%p: %p = %lu]\r\n", this, pointer, counter, *counter);
            }
        }
    }

private:
    // pointer to shared object
    T* pointer;

    // pointer to shared reference counter
    uint32_t* counter;
};

/** Non-member relational operators.
  */
template <class T, class U>
bool operator== (const SharedPointer<T>& lhs, const SharedPointer<U>& rhs) {
    return (lhs.get() == rhs.get());
}

template <class T, typename U>
bool operator== (const SharedPointer<T>& lhs, U rhs) {
    return (lhs.get() == (T*) rhs);
}

template <class T, typename U>
bool operator== (U lhs, const SharedPointer<T>& rhs) {
    return ((T*) lhs == rhs.get());
}

/** Non-member relational operators.
  */
template <class T, class U>
bool operator!= (const SharedPointer<T>& lhs, const SharedPointer<U>& rhs) {
    return (lhs.get() != rhs.get());
}

template <class T, typename U>
bool operator!= (const SharedPointer<T>& lhs, U rhs) {
    return (lhs.get() != (T*) rhs);
}

template <class T, typename U>
bool operator!= (U lhs, const SharedPointer<T>& rhs) {
    return ((T*) lhs != rhs.get());
}

} // namespace util
} // namespace mbed

#endif // __CORE_UTIL_SHAREDPOINTER_H__
