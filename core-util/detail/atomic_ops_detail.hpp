/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2015 ARM Limited
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

#ifndef CORE_UTIL_DETAIL_ATOMIC_OPS_DETAIL_H
#define CORE_UTIL_DETAIL_ATOMIC_OPS_DETAIL_H

#include <stdint.h>

#include "core-util/atomic_ops.h"
#include "core-util/CriticalSectionLock.h"


namespace mbed {
namespace util {

/******************************************************************************
 * Generic templates                                                          *
 ******************************************************************************/

template<typename T>
bool atomic_cas(T *ptr, T *expectedCurrentValue, T desiredValue)
{
    bool rc = true;

    CriticalSectionLock lock;

    T currentValue = *ptr;
    if (currentValue == *expectedCurrentValue) {
        *ptr = desiredValue;
    } else {
        *expectedCurrentValue = currentValue;
        rc = false;
    }

    return rc;
}

template<typename T>
T atomic_incr(T *valuePtr, T delta)
{
    T oldValue = *valuePtr;
    while (true) {
        const T newValue = oldValue + delta;
        if (atomic_cas(valuePtr, &oldValue, newValue)) {
            return newValue;
        }
    }
}

template<typename T>
T atomic_decr(T *valuePtr, T delta)
{
    T oldValue = *valuePtr;
    while (true) {
        const T newValue = oldValue - delta;
        if (atomic_cas(valuePtr, &oldValue, newValue)) {
            return newValue;
        }
    }
}

/******************************************************************************
 * Overloads                                                                  *
 ******************************************************************************/
/* For ARMv7-M and above, we use the load/store-exclusive instructions to
 * implement atomic_cas, so we provide three template specializations
 * corresponding to the byte, half-word, and word variants of the instructions.
 */
#if (__CORTEX_M >= 0x03)
bool atomic_cas(uint8_t *ptr, uint8_t *expectedCurrentValue, uint8_t desiredValue);
bool atomic_cas(uint16_t *ptr, uint16_t *expectedCurrentValue, uint16_t desiredValue);
bool atomic_cas(uint32_t *ptr, uint32_t *expectedCurrentValue, uint32_t desiredValue);

uint32_t atomic_incr(uint32_t *valuePtr, uint32_t delta);
uint16_t atomic_incr(uint16_t *valuePtr, uint16_t delta);
uint8_t atomic_incr(uint8_t *valuePtr, uint8_t delta);

uint32_t atomic_decr(uint32_t *valuePtr, uint32_t delta);
uint16_t atomic_decr(uint16_t *valuePtr, uint16_t delta);
uint8_t atomic_decr(uint8_t *valuePtr, uint8_t delta);


#endif /* #if (__CORTEX_M >= 0x03) */

} // namespace util
} // namespace mbed
#endif // CORE_UTIL_DETAIL_ATOMIC_OPS_DETAIL_H
