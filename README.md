# mbed utilities library

Implementation of various generic data structures and algorithms used in mbed.

# Configuration
Some parameters of the core-util library can be configured in yotta.  Currently, core-util supports configuring two things: the argument storage size of FunctionPointerBind and whether or not FunctionPointer checks its arguments before calling

## Configuring the storage size for FunctionPointerBind's bound arguments
In some cases it may be necessary to increase FunctionPointerBind's argument size.  In others, for memory optimization, it may be necessary to decrease the size of FunctionPointerBind's bound arguments.  If either of these are necessary, adding a new key with yotta config will allow this configuration: ```"util": {"functionPointer":{"arg-storage" : <bytes>}}```.

## Configuring whether or not FunctionPointer checks its arguments before calling
For debug purposes, it is possible to have FunctionPointer check its arguments before being called.   If it checks its arguments, it will use a ```CORE_UTIL_ASSERT```.  Checks can be disabled with: ```"util": {"functionPointer":{"disable-null-check" : true}}```
