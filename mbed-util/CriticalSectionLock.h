// Copyright (C) 2015 ARM Limited. All rights reserved.

#ifndef __MBED_UTIL_CRITICAL_SECTION_LOCK_H__
#define __MBED_UTIL_CRITICAL_SECTION_LOCK_H__

#include <stdint.h>
#include "cmsis-core/core_generic.h"
#ifdef TARGET_NORDIC
#include "nrf_soc.h"
#endif

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
#ifdef TARGET_NORDIC
        sd_nvic_critical_region_enter(&_state);
#else
        _state = __get_PRIMASK();
        __disable_irq();
#endif
    }

    ~CriticalSectionLock() {
#ifdef TARGET_NORDIC
        sd_nvic_critical_region_exit(_state);
#else
        __set_PRIMASK(_state);
#endif
    }

public:
#ifdef TARGET_NORDIC
    uint8_t  _state;
#else
    uint32_t _state;
#endif
};

} // namespace util
} // namespace mbed

#endif // #ifndef __MBED_UTIL_CRITICAL_SECTION_LOCK_H__

