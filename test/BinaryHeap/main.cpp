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

#include "core-util/BinaryHeap.h"
#include "greentea-client/test_env.h"
#include "mbed-drivers/mbed.h"
#include "unity/unity.h"
#include "utest/utest.h"
#include <stdio.h>
#include <stdlib.h>

using namespace utest::v1;
using namespace mbed::util;

template<typename T, typename Compare>
static void test_heap(const T* data, unsigned data_size, const T* sorted_data,
                      const T* to_remove, unsigned removed_size, const T* sorted_after_remove,
                      const T& not_in_heap) {
    BinaryHeap<T, Compare> heap;
    const size_t initial_capacity = data_size / 2, grow_capacity = (data_size * 3) / 2, alignment = 4;
    UAllocTraits_t traits = {0};
    TEST_ASSERT_TRUE(heap.init(initial_capacity, grow_capacity, traits, alignment));

   // Fill the heap with data
    for (unsigned i = 0; i < data_size; i++) {
        heap.insert(data[i]);
        TEST_ASSERT_TRUE(heap.is_consistent());
    }
    TEST_ASSERT_EQUAL(data_size, heap.get_num_elements());

    // Remove and check root at each step
    for (unsigned i = 0; i < data_size; i ++) {
        T root = heap.get_root();
        TEST_ASSERT_TRUE(root == sorted_data[i]);
        heap.remove_root();
        TEST_ASSERT_TRUE(heap.is_consistent());
    }
    TEST_ASSERT_TRUE(heap.is_empty());

    // Put everything back again
    for (unsigned i = 0; i < data_size; i++) {
        heap.insert(data[i]);
        TEST_ASSERT_TRUE(heap.is_consistent());
    }
    TEST_ASSERT_EQUAL(data_size, heap.get_num_elements());

    // And check removing
    for (unsigned i = 0; i < removed_size; i ++) {
        TEST_ASSERT_TRUE(heap.remove(to_remove[i]));
        TEST_ASSERT_TRUE(heap.is_consistent());
    }
    TEST_ASSERT_TRUE(!heap.remove(not_in_heap)); // this element is not in the heap
    TEST_ASSERT_EQUAL(data_size - removed_size, heap.get_num_elements());
    // Remove and check root at each step
    for (unsigned i = 0; i < data_size - removed_size; i ++) {
        T root = heap.pop_root();
        TEST_ASSERT_TRUE(root == sorted_after_remove[i]);
        TEST_ASSERT_TRUE(heap.is_consistent());
    }
    TEST_ASSERT_TRUE(heap.is_empty());
    TEST_ASSERT_EQUAL(0, heap.get_num_elements());
}

static void test_min_heap_pod() {
    int data[] = {20, 13, 8, 7, 100, -50, 0, 16, 1000, 2};
    int sorted_data[] = {-50, 0, 2, 7, 8, 13, 16, 20, 100, 1000};
    int to_remove[] = {-50, 100, 8, 2};
    int sorted_after_remove[] = {0, 7, 13, 16, 20, 1000};

    printf("********** Starting test_min_heap_pod()\r\n");
    test_heap<int, MinCompare<int> >(data, sizeof(data)/sizeof(int), sorted_data,
                   to_remove, sizeof(to_remove)/sizeof(int), sorted_after_remove,
                   2000);
    printf("********** Ending test_min_heap_pod()\r\n");
}

static void test_max_heap_pod() {
    unsigned data[] = {53, 0, 21, 19, 77, 123, 81, 0, 1001, 66, 17, 5};
    unsigned sorted_data[] = {1001, 123, 81, 77, 66, 53, 21, 19, 17, 5, 0, 0};
    unsigned to_remove[] = {0, 21, 66, 5, 81, 1001};
    unsigned sorted_after_remove[] = {123, 77, 53, 19, 17, 0};

    printf("********** Starting test_max_heap_pod()\r\n");
    test_heap<unsigned, MaxCompare<unsigned> >(data, sizeof(data)/sizeof(unsigned), sorted_data,
                   to_remove, sizeof(to_remove)/sizeof(unsigned), sorted_after_remove,
                   2000);
    printf("********** Ending test_max_heap_pod()\r\n");
}

struct Test {
    Test(int a = 0, uint8_t c = 10): _a(a), _c(c) {
        inst_count ++;
    }

    Test(const Test& t): _a(t._a), _c(t._c) {
        inst_count ++;
    }

    ~Test() {
        inst_count --;
    }

    bool operator ==(const Test& t) const {
        return (t._a == _a) && (t._c == _c);
    }

    bool operator <=(const Test& t) const {
        return _a <= t._a;
    }

    bool operator >=(const Test& t) const {
        return _a >= t._a;
    }

    int _a;
    uint8_t _c;
    static int inst_count;
};
int Test::inst_count = 0;

static void test_min_heap_non_pod() {
    {
    Test data[] = {20, 13, 8, 7, 100, -50, 0, 16, 1000, 2};
    Test sorted_data[] = {-50, 0, 2, 7, 8, 13, 16, 20, 100, 1000};
    Test to_remove[] = {-50, 100, 8, 2};
    Test sorted_after_remove[] = {0, 7, 13, 16, 20, 1000};

    printf("********** Starting test_min_heap_non_pod()\r\n");
    test_heap<Test, MinCompare<Test> >(data, sizeof(data)/sizeof(Test), sorted_data,
                   to_remove, sizeof(to_remove)/sizeof(Test), sorted_after_remove,
                   2000);
    }
    TEST_ASSERT_EQUAL(0, Test::inst_count);
    printf("********** Ending test_min_heap_non_pod()\r\n");
}

static void test_max_heap_non_pod() {
    {
    Test data[] = {291, 62, 364, 63, 753, 325, -382, -736, -930, -927, 734, -591, 136, 753, 576, -59, -930, -700, -380, 764};
    Test sorted_data[] = {764, 753, 753, 734, 576, 364, 325, 291, 136, 63, 62, -59, -380, -382, -591, -700, -736, -927, -930, -930};
    Test to_remove[] = {-927, 576, 63, 753, -930, 364, 753}; // this will remove two elements with the same value
    Test sorted_after_remove[] = {764, 734, 325, 291, 136, 62, -59, -380, -382, -591, -700, -736, -930};

    printf("********** Starting test_max_heap_non_pod()\r\n");
    test_heap<Test, MaxCompare<Test> >(data, sizeof(data)/sizeof(Test), sorted_data,
                   to_remove, sizeof(to_remove)/sizeof(Test), sorted_after_remove,
                   2000);
    }
    TEST_ASSERT_EQUAL(0, Test::inst_count);
    printf("********** Ending test_max_heap_non_pod()\r\n");
}

static status_t test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(5, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

static Case cases[] = {
    Case("BinaryHeap  - test_min_heap_pod", test_min_heap_pod, greentea_failure_handler),
    Case("BinaryHeap  - test_max_heap_pod", test_max_heap_pod, greentea_failure_handler),
    Case("BinaryHeap  - test_min_heap_non_pod", test_min_heap_non_pod, greentea_failure_handler),
    Case("BinaryHeap  - test_max_heap_non_pod", test_max_heap_non_pod, greentea_failure_handler)
};

static Specification specification(test_setup, cases, greentea_test_teardown_handler);

void app_start(int, char**) {
    Harness::run(specification);
}

