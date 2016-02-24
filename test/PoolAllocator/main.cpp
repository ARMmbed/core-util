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

#include "core-util/PoolAllocator.h"
#include "greentea-client/test_env.h"
#include "unity/unity.h"
#include "utest/utest.h"
#include <stdio.h>
#include <stdlib.h>

using namespace utest::v1;
using namespace mbed::util;

void test_pool_allocator() {
    // Allocate initial space for the pool
    const size_t elements = 10, element_size = 6;
    const size_t aligned_size = (element_size + MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN - 1) & ~(MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN - 1);
    size_t pool_size = PoolAllocator::get_pool_size(elements, element_size);
    TEST_ASSERT_EQUAL(elements * aligned_size, pool_size);

    void *start = malloc(pool_size);
    TEST_ASSERT_TRUE(start != NULL);
    PoolAllocator allocator(start, elements, element_size);

    // Allocate all elements, checking for proper alignment and spacing
    void *p, *prev, *first;
    for (size_t i = 0; i < elements; i ++) {
        p = allocator.alloc();
        TEST_ASSERT_TRUE(p != NULL);
        // Check alignment
        TEST_ASSERT_EQUAL(0, ((uint32_t)p & (MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN - 1)));
        // Check spacing
        if (i > 0) {
            TEST_ASSERT_EQUAL(aligned_size, ((uint32_t)p - (uint32_t)prev));
        } else {
            first = p;
            TEST_ASSERT_EQUAL(start, p);
        }
        prev = p;
    }

    // No more space in the pool, we should get NULL now
    TEST_ASSERT_EQUAL(NULL, allocator.alloc());

    // Free the first element we allocated
    allocator.free(first);

    // Verify that we can allocate a single element now, and it has the same address
    // as the first element we allocated above
    p = allocator.alloc();
    TEST_ASSERT_EQUAL(first, p);
    p = allocator.alloc();
    TEST_ASSERT_EQUAL(NULL, p);
}

static status_t test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(5, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

static Case cases[] = {
    Case("PoolAllocator  - test_pool_allocator", test_pool_allocator)
};

static Specification specification(test_setup, cases, greentea_test_teardown_handler);

void app_start(int, char**) {
    Harness::run(specification);
}
