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

#include <stddef.h>
#include <stdint.h>
#include "greentea-client/test_env.h"
#include "unity/unity.h"
#include "utest/utest.h"
#include "core-util/SharedPointer.h"

using namespace utest::v1;
using namespace mbed::util;

static bool globalFlag = false;

class Number {
public:
    Number(int _num): num(_num) {
        printf("Number: %d constructed\r\n", num);
    }

    ~Number() {
        printf("Number: %d destroyed\r\n", num);
        globalFlag = false;
    }

    int getNum() {
        return num;
    }
private:
    int num;
};


void test_shared_pointer() {
    /* Test 1: create SharedPointer from pointer */
    Number* ptr1 = new Number(1);
    SharedPointer<Number> sharedptr1(ptr1);

    // pointers match
    TEST_ASSERT_TRUE(sharedptr1.get() == ptr1);

    // counter match
    TEST_ASSERT_TRUE(sharedptr1.use_count() == 1);

    // dereferencing works
    TEST_ASSERT_TRUE(sharedptr1->getNum() == 1);

    /* Test 2: copy pointer */
    {
        SharedPointer<Number> sharedptr1copy;

        // NULL pointer is NULL
        TEST_ASSERT_TRUE(sharedptr1copy.get() == NULL);
        TEST_ASSERT_TRUE(sharedptr1copy.use_count() == 0);

        // copy
        sharedptr1copy = sharedptr1;

        // raw pointer is consistent
        TEST_ASSERT_TRUE(sharedptr1copy.get() == ptr1);
        TEST_ASSERT_TRUE(sharedptr1copy.get() == sharedptr1.get());

        // reference count is accurate
        TEST_ASSERT_TRUE(sharedptr1.use_count() == 2);
        TEST_ASSERT_TRUE(sharedptr1copy.use_count() == 2);
    }

    // sharedptr1copy is destroyed, count is decremented
    TEST_ASSERT_TRUE(sharedptr1.use_count() == 1);


    /* Test 3: object is destroyed */
    {
        SharedPointer<Number> sharedptr2(new Number(2));

        // Set global variable. This variable is set to false when any Number is destroyed.
        globalFlag = true;
    }

    TEST_ASSERT_TRUE(globalFlag == false);

    /* Test 4: corner cases */

    // self assignment
    TEST_ASSERT_TRUE(sharedptr1.use_count() == 1);
    sharedptr1 = sharedptr1;
    TEST_ASSERT_TRUE(sharedptr1.use_count() == 1);


    /* Test 5: basic types */
    {
        SharedPointer<int> sharedptr3(new int);
        *sharedptr3 = 3;
        TEST_ASSERT_TRUE(*sharedptr3 == 3);
    }
}

static status_t test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(2, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

static Case cases[] = {
    Case("SharedPointer  - test_shared_pointer", test_shared_pointer)
};

static Specification specification(test_setup, cases, greentea_test_teardown_handler);

void app_start(int, char**) {
    Harness::run(specification);
}
