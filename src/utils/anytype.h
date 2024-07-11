#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "types.h"

class GyverDB;

namespace gdb {

class AnyType {
#ifndef DB_NO_INT64
    uint64_t buf = 0;
#else
    uint32_t buf = 0;
#endif

   public:
    AnyType(const GyverDB& value) = delete;

    template <typename T>
    AnyType(const T& value) : ptr(&value), len(sizeof(T)), type(Type::Bin) {}
    AnyType(const void* ptr, size_t len) : ptr(ptr), len(len), type(Type::Bin) {}

    AnyType(const char* str, size_t len) : ptr(str), len(len), type(Type::String) {}
    AnyType(const char* str) : AnyType(str, strlen(str)) {}
    AnyType(const String& str) : AnyType(str.c_str(), str.length()) {}
    AnyType(const Text& str) : AnyType(str.str(), str.length()) {}

    AnyType(signed char val) : AnyType((long)val) {}
    AnyType(short val) : AnyType((long)val) {}
    AnyType(int val) : AnyType((long)val) {}
    AnyType(long val) : buf(val), ptr(&buf), len(4), type(Type::Int) {}
#ifndef DB_NO_INT64
    AnyType(long long val) : buf(val), ptr(&buf), len(8), type(Type::Int64) {}
#endif

    AnyType(bool val) : AnyType((unsigned long)val) {}
    AnyType(char val) : AnyType((unsigned long)val) {}
    AnyType(unsigned char val) : AnyType((unsigned long)val) {}
    AnyType(unsigned short val) : AnyType((unsigned long)val) {}
    AnyType(unsigned int val) : AnyType((unsigned long)val) {}
    AnyType(unsigned long val) : buf(val), ptr(&buf), len(4), type(Type::Uint) {}
#ifndef DB_NO_INT64
    AnyType(unsigned long long val) : buf(val), ptr(&buf), len(8), type(Type::Uint64) {}
#endif

    AnyType(float val) : buf(*((uint32_t*)&val)), ptr(&buf), len(4), type(Type::Float) {}
    AnyType(double val) : AnyType((float)val) {}

    const void* ptr = nullptr;
    size_t len = 0;
    Type type = Type::None;
};

}  // namespace gdb
