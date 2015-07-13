// Copyright (C) 2015 ARM Limited. All rights reserved.

#ifndef __MBED_UTIL_CRITICAL_SECTION_LOCK_H__
#define __MBED_UTIL_CRITICAL_SECTION_LOCK_H__

#include <stdint.h>
#include "cmsis-core/core_generic.h"

namespace mbed {
namespace util {

/** RAII object for disabling, then restoring, interrupt state
  * Usage:
  * @code
  *
  * void f() {
  *     // some code here
  *     {
  *         CriticalSectionLock lock;
  *         // Code in this block will run with interrupts disabled
  *     }
  *     // interrupts will be restored to their previous state
  * }
  * @endcode
  */
class CriticalSectionLock {
public:
    CriticalSectionLock() {
        _state = __get_PRIMASK();
        __disable_irq();
    }

    ~CriticalSectionLock() {
        __set_PRIMASK(_state);
    }

private:
    uint32_t _state;
};

} // namespace util
} // namespace mbed

#endif // #ifndef __MBED_UTIL_CRITICAL_SECTION_LOCK_H__

