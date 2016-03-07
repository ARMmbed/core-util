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

#include "core-util/ExtendablePoolAllocator.h"
#include "greentea-client/test_env.h"
#include "unity/unity.h"
#include "utest/utest.h"
#include "ualloc/ualloc.h"
#include <stdio.h>
#include <stdlib.h>

using namespace utest::v1;
using namespace mbed::util;

static bool check_value_and_alignment(void *p, unsigned alignment = MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN) {
    if (NULL == p)
        return false;
    return ((uint32_t)p & (alignment - 1)) == 0;
}

static void test_extendable_pool_allocator() {
    // Create the allocator
    const size_t initial_elements = 20, new_pool_elements = 12, element_size = 6;
    UAllocTraits_t traits = {0};
    ExtendablePoolAllocator allocator;
    TEST_ASSERT_TRUE(allocator.init(initial_elements, new_pool_elements, element_size, traits));

    // Fill the first pool
    for (unsigned i = 0; i < initial_elements; i ++) {
        TEST_ASSERT_TRUE(check_value_and_alignment(allocator.alloc()));
    }
    TEST_ASSERT_EQUAL(1, allocator.get_num_pools());

    // Allocating just another element should add another pool
    void *p = allocator.alloc();
    TEST_ASSERT_TRUE(check_value_and_alignment(p));
    TEST_ASSERT_EQUAL(2, allocator.get_num_pools());
    // Fill the new pool
    for (unsigned i = 0; i < new_pool_elements - 1; i ++) {
        TEST_ASSERT_TRUE(check_value_and_alignment(allocator.alloc()));
    }
    TEST_ASSERT_EQUAL(2, allocator.get_num_pools());

    // Free one previously allocated area
    allocator.free(p);
    // And allocate again. Since there's a single free place, it should be used now
    void *newp = allocator.alloc();
    TEST_ASSERT_TRUE(check_value_and_alignment(newp));
    TEST_ASSERT_EQUAL(p, newp);
    TEST_ASSERT_EQUAL(2, allocator.get_num_pools());
}

static status_t test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(5, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

static Case cases[] = {
    Case("ExtendablePoolAllocator  - test_extendable_pool_allocator", test_extendable_pool_allocator)
};

static Specification specification(test_setup, cases, greentea_test_teardown_handler);

void app_start(int, char**) {
    Harness::run(specification);
}
