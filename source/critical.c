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
#include <stdint.h>                  // uint32_t, UINT32_MAX
#include <stddef.h>                  // NULL
#include "cmsis-core/core_generic.h" //__disable_irq, __enable_irq

// Module include
#include "mbed-util/critical.h"

static volatile uint32_t interruptEnableCounter = 0;
static volatile uint32_t critical_primask = 0;
void critical_section_enter()
{
    /* sample the primask */
    if (!interruptEnableCounter) {
        critical_primask = __get_PRIMASK();
    }
    __disable_irq();
    /* not allowed to overflow the interruptEnableCounter. */
    if (interruptEnableCounter == UINT32_MAX) {
        /* Generate a fault */
        *((uint32_t *)NULL) = 0;
    }
    interruptEnableCounter++;
}

void critical_section_exit()
{
    if (interruptEnableCounter) interruptEnableCounter--;
    if (!interruptEnableCounter && !critical_primask) __enable_irq();
}
