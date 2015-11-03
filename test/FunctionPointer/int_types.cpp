/*
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

#include <stdio.h>
#include "core-util/FunctionPointer.h"
#include "int_types.hpp"

bool called;

static void vsi(int i)
{
    called=true;
    printf("%s called with %i\r\n", __PRETTY_FUNCTION__, i);
}
static void vspi(int *pi)
{
    called=true;
    printf("%s called with %i\r\n", __PRETTY_FUNCTION__, *pi);
}
static void vsri(int& ri)
{
    called=true;
    printf("%s called with %i\r\n", __PRETTY_FUNCTION__, ri);
}
static void vsci(const int i)
{
    called=true;
    printf("%s called with %i\r\n", __PRETTY_FUNCTION__, i);
}
static void vscpi(const int *pi)
{
    called=true;
    printf("%s called with %i\r\n", __PRETTY_FUNCTION__, *pi);
}
static void vscri(const int& ri)
{
    called=true;
    printf("%s called with %i\r\n", __PRETTY_FUNCTION__, ri);
}

bool testInts(){
    bool passed = true;
    {
        mbed::util::FunctionPointer1<void,int> fp_vsi(vsi);
        called = false;
        fp_vsi(1);
        passed = called && passed;
    }
    {
        mbed::util::FunctionPointer1<void,int *> fp_vspi(vspi);
        called = false;
        int i = 2;
        fp_vspi(&i);
        passed = called && passed;
    }
    {
        mbed::util::FunctionPointer1<void,int &> fp_vsri(vsri);
        called = false;
        int i = 3;
        fp_vsri(i);
        passed = called && passed;
    }
    {
        mbed::util::FunctionPointer1<void,const int> fp_vsci(vsci);
        called = false;
        fp_vsci(4);
        passed = called && passed;
    }
    {
        mbed::util::FunctionPointer1<void,const int *> fp_vscpi(vscpi);
        called = false;
        int i = 5;
        fp_vscpi(&i);
        passed = called && passed;
    }
    {
        mbed::util::FunctionPointer1<void,const int &> fp_vscri(vscri);
        called = false;
        int i = 6;
        fp_vscri(i);
        passed = called && passed;
    }
    return passed;
}
