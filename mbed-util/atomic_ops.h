// Copyright (C) 2015 ARM Limited. All rights reserved.


#ifndef __MBED_UTIL_ATOMIC_OPS_H__
#define __MBED_UTIL_ATOMIC_OPS_H__

#include <stdint.h>
#include "cmsis-core/core_generic.h"

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
 * Refer to https://en.wikipedia.org/wiki/Compare-and-set [which may redirect
 * you to the article on compare-and swap].
 *
 * @param  ptr                  The target memory location.
 * @param[in/out] expectedCurrentValue The expected current value of the data at the location.
 *                              The computed 'desiredValue' should be a function of this current value.
 *                              @Note: This is an in-out parameter. In the
 *                              failure case of atomic_cas (where the
 *                              destination isn't set), expectedCurrentValue is
 *                              updated with the current value.
 * @param[in] desiredValue      The new value computed based on 'expectedCurrentValue'.
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
 * For Cortex-M3 and above, we use the load/store-exclusive instructions to
 * implement atomic_cas, so we provide three overloaded versions corresponding
 * to the byte, half-word, and word variants of the instructions; for Cortex-M0,
 * synchronization requires blocking interrupts, and we have the liberty of
 * creating a generic templatized variant of atomic_cas.
 */
#if (__CORTEX_M >= 0x03)
bool atomic_cas(uint8_t  *ptr, uint8_t  &expectedCurrentValue, uint8_t  desiredValue);
bool atomic_cas(uint16_t *ptr, uint16_t &expectedCurrentValue, uint16_t desiredValue);
bool atomic_cas(uint32_t *ptr, uint32_t &expectedCurrentValue, uint32_t desiredValue);
#elif (__CORTEX_M == 0x00)
template<typename T>
bool atomic_cas(T *ptr, T &expectedCurrentValue, T desiredValue)
{
    bool rc = true;

    uint32_t previousPRIMASK = __get_PRIMASK();
    __disable_irq();

    T currentValue = *ptr;
    if (currentValue == expectedCurrentValue) {
        *ptr = desiredValue;
    } else {
        expectedCurrentValue = currentValue;
        rc = false;
    }

    if (!previousPRIMASK) {
        __enable_irq();
    }

    return rc;
}
#endif /* #if (__CORTEX_M == 0x00) */

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
