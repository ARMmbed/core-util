// Copyright (C) 2015 ARM Limited. All rights reserved.


#ifndef __MBED_UTIL_ATOMIC_OPS_H__
#define __MBED_UTIL_ATOMIC_OPS_H__

#include <stdint.h>

namespace mbed {
namespace util {

/**
 * Atomic compare and set. It compares the contents of a memory location to a
 * given value and, only if they are the same, modifies the contents of that
 * memory location to a given new value. This is done as a single atomic
 * operation. The atomicity guarantees that the new value is calculated based on
 * up-to-date information; if the value had been updated by another thread in
 * the meantime, the write would fail due to a mismatched expectedCurrentValue.
 *
 * Refer to https://en.wikipedia.org/wiki/Compare-and-swap
 *
 * @param  ptr                  The target memory location.
 * @param  expectedCurrentValue The expected current value of the data at the location.
 *                              This computed 'desiredValue' should be a function of this current value.
 * @param  desiredValue         The new value computed based on 'expectedCurrentValue'.
 *
 * @return                      true if the memory location was atomically
 *                              updated with the desired value (after verifying
 *                              that it contained the expectedCurrentValue),
 *                              false otherwise. In the failure case,
 *                              exepctedCurrentValue is updated with the new
 *                              value of the target memory location.
 *
 * pseudocode:
 * function cas(p : pointer to int, old : int, new : int) returns bool {
 *     if *p != old {
 *         old = *p
 *         return false
 *     }
 *     *p = new
 *     return true
 * }
 *
 * @Note: In the failure case (where the destination isn't set),
 * expectedCurrentValue is still updated with the current value. This property
 * helps writing concise code for the following incr:
 *
 * function incr(p : pointer to int, a : int) returns int {
 *     done = false
 *     value = *p  // This fetch operation need not be atomic.
 *     while not done {
 *         done = atomic_cas(p, value, value + a) // value gets updated automatically until success
 *     }
 *     return value + a
 * }
 *
 * We implement three overloaded versions of atomic_cas to correspond with byte,
 * half-word, and word instructions.
 */
bool atomic_cas(uint8_t  *ptr, uint8_t  &expectedCurrentValue, uint8_t  desiredValue);
bool atomic_cas(uint16_t *ptr, uint16_t &expectedCurrentValue, uint16_t desiredValue);
bool atomic_cas(uint32_t *ptr, uint32_t &expectedCurrentValue, uint32_t desiredValue);

/**
 * Atomic increment.
 * @param  valuePtr Target memory location being incremented.
 * @param  delta    The amount being incremented.
 * @return          The new incremented value.
 */
template<typename T> T atomic_incr(T *valuePtr, T delta)
{
    T oldValue = *valuePtr;
    while (true) {
        const T newValue = oldValue + delta;
        if (atomic_cas(valuePtr, oldValue, newValue)) {
            return newValue;
        }
    }
}

/**
 * Atomic decrement.
 * @param  valuePtr Target memory location being decremented.
 * @param  delta    The amount being decremented.
 * @return          The new decremented value.
 */
template<typename T> T atomic_decr(T *valuePtr, T delta)
{
    T oldValue = *valuePtr;
    while (true) {
        const T newValue = oldValue - delta;
        if (atomic_cas(valuePtr, oldValue, newValue)) {
            return newValue;
        }
    }
}

} // namespace util
} // namespace mbed

#endif // #ifndef __MBED_UTIL_ATOMIC_OPS_H__
