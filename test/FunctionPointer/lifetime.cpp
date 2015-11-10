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
#include "lifetime.hpp"
#include "core-util/FunctionPointer.h"

class LifetimeChecker {
public:
    LifetimeChecker(int arg) : _arg(arg) {
        _activeInstances++;
    }
    LifetimeChecker(const LifetimeChecker & lc) : _arg(lc._arg) {
        _cc = true;
        _activeInstances++;
    }
    ~LifetimeChecker() {
        _activeInstances--;
    }

    LifetimeChecker& operator = (const LifetimeChecker & rhs) {
        _cc = true;
        _arg = rhs._arg;
        return *this;
    }

    static int getInstances() {
        return _activeInstances;
    }
    int getArg() {
        return _arg;
    }
    static void reset() {
        _wasCalled = false;
        _ok = false;
        _cc = false;
    }
    static bool wasCalled() {
        return _wasCalled;
    }
    static bool isOk() {
        return _ok;
    }
    static bool ccCalled() {
        return _cc;
    }
    void set(bool ok) {
        _wasCalled = true;
        _ok = ok;
    }
protected:
    int _arg;
    static bool _wasCalled;
    static bool _ok;
    static bool _cc;
    static int _activeInstances;
};

int LifetimeChecker::_activeInstances = 0;
bool LifetimeChecker::_wasCalled;
bool LifetimeChecker::_ok;
bool LifetimeChecker::_cc;

int lifeCheck(LifetimeChecker lc) {
    int arg = lc.getArg();
    int instances = lc.getInstances();
    lc.set(instances == 4);
    if (instances != 4) {
        printf("Expected 4 instances, got %i\r\n",instances);
    }
    return arg-1;
}
int lifeCheckR(LifetimeChecker &lc) {
    int arg = lc.getArg();
    int instances = lc.getInstances();
    lc.set(instances == 1);
    if (instances != 1) {
        printf("Expected 1 instances, got %i\r\n",instances);
    }
    return arg-2;
}
int lifeCheckP(LifetimeChecker *lc) {
    int arg = lc->getArg();
    int instances = lc->getInstances();
    lc->set(instances == 1);
    if (instances != 1) {
        printf("Expected 1 instances, got %i\r\n",instances);
    }
    return arg-3;
}


bool checkLifetime() {
    bool passed = true;
    int rc;
    {
        LifetimeChecker::reset();
        LifetimeChecker lc(1);
        mbed::util::FunctionPointer1<int, LifetimeChecker> fp(lifeCheck);
        if ((rc = fp(lc)) != 0) {
            printf("call error: expected 0, got %i\n", rc);
            passed = false;
        }
        printf("LifetimeChecker    : OK = %s, Was Called = %s, Copy Constructed = %s, Instances remaining: %d\r\n",
            (lc.isOk()?"true":"false"), (lc.wasCalled()?"true":"false"), (lc.ccCalled()?"true":"false"), lc.getInstances());
        passed = passed && lc.isOk() && lc.wasCalled() && lc.ccCalled() && (lc.getInstances() == 1);
    }
    {
        LifetimeChecker::reset();
        LifetimeChecker lc(2);
        mbed::util::FunctionPointer1<int, LifetimeChecker&> fp(lifeCheckR);
        if ((rc = fp(lc)) != 0) {
            printf("call error: expected 0, got %i\n", rc);
            passed = false;
        }
        printf("LifetimeChecker (R): OK = %s, Was Called = %s, Copy Constructed = %s, Instances remaining: %d\r\n",
            (lc.isOk()?"true":"false"), (lc.wasCalled()?"true":"false"), (lc.ccCalled()?"true":"false"), lc.getInstances());
        passed = passed && lc.isOk() && lc.wasCalled() && !lc.ccCalled() && (lc.getInstances() == 1);
    }
    {
        LifetimeChecker::reset();
        LifetimeChecker lc(3);
        mbed::util::FunctionPointer1<int, LifetimeChecker*> fp(lifeCheckP);
        if ((rc = fp(&lc)) != 0) {
            printf("call error: expected 0, got %i\n", rc);
            passed = false;
        }
        printf("LifetimeChecker (P): OK = %s, Was Called = %s, Copy Constructed = %s, Instances remaining: %d\r\n",
            (lc.isOk()?"true":"false"), (lc.wasCalled()?"true":"false"), (lc.ccCalled()?"true":"false"), lc.getInstances());
        passed = passed && lc.isOk() && lc.wasCalled() && !lc.ccCalled() && (lc.getInstances() == 1);
    }
    return passed;
}
