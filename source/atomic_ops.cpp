#include "mbed-util/atomic_ops.h"
#include "cmsis-core/core_generic.h"

/* The following macros use IRQ masking to simulate LDREX and STREX on a CM0. Their use
 * requires declaring a variable called previousPRIMASK within the enclosing
 * scope. The following are needed because CM0 doesn't support LDREX and STREX. */
#if (__CORTEX_M == 0x00)
#define __LDREXW(ADDR)        (previousPRIMASK = __get_PRIMASK(), __disable_irq(), *(ADDR))
#define __CLREX()             if (!previousPRIMASK) __enable_irq()
#define __STREXW(VALUE, ADDR) (*(ADDR) = VALUE, (previousPRIMASK || (__enable_irq(), true /* turn this into a boolean */)), 0 /* simulate successful STREX */)

#define __LDREXB(ADDR)        __LDREXW(ADDR)
#define __STREXB(VALUE, ADDR) __STREXW(VALUE, ADDR)
#define __LDREXH(ADDR)        __LDREXW(ADDR)
#define __STREXH(VALUE, ADDR) __STREXW(VALUE, ADDR)
#endif /* (__CORTEX_M == 0x00) */

bool mbed::util::atomic_cas(uint8_t *ptr, uint8_t &expectedCurrentValue, uint8_t desiredValue)
{
#if (__CORTEX_M == 0x00)
    uint32_t previousPRIMASK;
#endif

    uint8_t currentValue = __LDREXB(ptr);
    if (currentValue != expectedCurrentValue) {
        expectedCurrentValue = currentValue;
        __CLREX();
        return false;
    }

    return !__STREXB(desiredValue, ptr);
}

bool mbed::util::atomic_cas(uint16_t *ptr, uint16_t &expectedCurrentValue, uint16_t desiredValue)
{
#if (__CORTEX_M == 0x00)
    uint32_t previousPRIMASK;
#endif

    uint16_t currentValue = __LDREXH(ptr);
    if (currentValue != expectedCurrentValue) {
        expectedCurrentValue = currentValue;
        __CLREX();
        return false;
    }

    return !__STREXH(desiredValue, ptr);
}

bool mbed::util::atomic_cas(uint32_t *ptr, uint32_t &expectedCurrentValue, uint32_t desiredValue)
{
#if (__CORTEX_M == 0x00)
    uint32_t previousPRIMASK;
#endif

    uint32_t currentValue = __LDREXW(ptr);
    if (currentValue != expectedCurrentValue) {
        expectedCurrentValue = currentValue;
        __CLREX();
        return false;
    }

    return !__STREXW(desiredValue, ptr);
}
