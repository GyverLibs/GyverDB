#pragma once
#include <Arduino.h>
#include <FOR_MACRO.h>
#include <StringUtils.h>

#define DB_TYPE_SIZE (3ul)
#define DB_HASH_SIZE (32ul - DB_TYPE_SIZE)
#define DB_TYPE_MASK (((1ul << DB_TYPE_SIZE) - 1) << DB_HASH_SIZE)
#define DB_HASH_MASK ((1ul << DB_HASH_SIZE) - 1)
#define DB_GET_TYPE(x) (Type)((uint32_t)x & DB_TYPE_MASK)
#define DB_GET_HASH(x) ((uint32_t)x & DB_HASH_MASK)
#define DB_MAKE_TYPEHASH(t, h) ((uint32_t)t | (h & DB_HASH_MASK))
#define DB_REPLACE_TYPE(x, t) ((x & ~(DB_TYPE_MASK)) | (uint32_t)t)

// #define DB_KEY(name) name = SH(#name) & DB_HASH_MASK
// #define DB_KEYS(name, ...) enum name : size_t { __VA_ARGS__ };

#define DB_KEY(name) name
#define _DB_KEY(N, i, p, val) val = (SH(#val) & DB_HASH_MASK),

#define DB_KEYS(name, ...) enum name : size_t { FOR_MACRO(_DB_KEY, 0, __VA_ARGS__) };

namespace gdb {

enum class Type : uint32_t {
    None = (0ul << DB_HASH_SIZE),
    Int = (1ul << DB_HASH_SIZE),
    Uint = (2ul << DB_HASH_SIZE),
    Int64 = (3ul << DB_HASH_SIZE),
    Uint64 = (4ul << DB_HASH_SIZE),
    Float = (5ul << DB_HASH_SIZE),
    String = (6ul << DB_HASH_SIZE),
    Bin = (7ul << DB_HASH_SIZE),
};

class Converter {
   public:
    Converter() {}
    Converter(Type type, const void* p, size_t len) : type(type), p(p), len(len) {}

    static bool isDynamic(Type type) {
        switch (type) {
            case Type::Int64:
            case Type::Uint64:
            case Type::Bin:
            case Type::String:
                return 1;

            default:
                break;
        }
        return 0;
    }

    static size_t size(Type type) {
        switch (type) {
            case Type::Int:
            case Type::Uint:
            case Type::Float:
                return 4;

            case Type::Int64:
            case Type::Uint64:
                return 8;

            default:
                break;
        }
        return 0;
    }

    String toString() const {
        if (!p) return String();
        switch (type) {
            case Type::Int:
            case Type::Int64:
                return String(toInt());

            case Type::Uint:
            case Type::Uint64:
                return String((uint32_t)toInt());

            case Type::String:
                return Text((const char*)p, len).toString();

#ifndef DB_NO_FLOAT
            case Type::Float:
                return String(toFloat());
#endif
            default:
                break;
        }
        return String();
    }

    bool toBool() const {
        if (!p) return 0;
        switch (type) {
            case Type::String: return (*(char*)p == 't' || *(char*)p == '1');
            default: break;
        }
        return toInt();
    }

    int32_t toInt() const {
        if (!p) return 0;
        switch (type) {
            case Type::Int:
            case Type::Uint:
                return *((int32_t*)p);
#ifndef DB_NO_INT64
            case Type::Int64:
            case Type::Uint64:
                return *((int64_t*)p);
#endif
            case Type::Float:
                return *((float*)p);

            case Type::String:
                return Text((const char*)p, len).toInt32();

            default:
                break;
        }
        return 0;
    }

    int16_t toInt16() const {
        return toInt();
    }

    int32_t toInt32() const {
        return toInt();
    }

    int64_t toInt64() const {
        if (!p) return 0;
        switch (type) {
#ifndef DB_NO_INT64
            case Type::Int64:
            case Type::Uint64:
                return *((int64_t*)p);
#endif
            case Type::String:
                return Text((const char*)p, len).toInt64();

            default:
                break;
        }
        return toInt();
    }

    float toFloat() const {
        if (!p) return 0;
        switch (type) {
            case Type::Float: return *((float*)p);
#ifndef DB_NO_FLOAT
            case Type::String: return Text((const char*)p, len).toFloat();
#endif
            default: break;
        }
        return toInt();
    }

    Value toText() const {
        if (!p) return Value();
        switch (type) {
            case Type::Int: return (int32_t)toInt();
            case Type::Uint: return (uint32_t)toInt();
#ifndef DB_NO_INT64
            case Type::Int64: return *((int64_t*)p);
            case Type::Uint64: return *((uint64_t*)p);
#endif
#ifndef DB_NO_FLOAT
            case Type::Float: return *((float*)p);
#endif
            case Type::String: return Text((const char*)p, len);
            default: break;
        }
        return Value();
    }

   private:
    Type type = Type::None;
    const void* p = nullptr;
    size_t len = 0;
};

}  // namespace gdb