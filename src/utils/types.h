#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#define DB_TYPE_SIZE (4ul)
#define DB_HASH_SIZE (32ul - DB_TYPE_SIZE)
#define DB_TYPE_MASK (((1ul << DB_TYPE_SIZE) - 1) << DB_HASH_SIZE)
#define DB_HASH_MASK ((1ul << DB_HASH_SIZE) - 1)
#define DB_GET_TYPE(x) (Type)((uint32_t)x & DB_TYPE_MASK)
#define DB_GET_HASH(x) ((uint32_t)x & DB_HASH_MASK)
#define DB_MAKE_TYPEHASH(t, h) ((uint32_t)t | (h & DB_HASH_MASK))
#define DB_REPLACE_TYPE(x, t) ((x & ~(DB_TYPE_MASK)) | (uint32_t)t)

#define DB_KEY(name) name = SH(#name)
#define DB_KEYS(name, ...) enum name : size_t { __VA_ARGS__ };

namespace gdb {

enum class Type : uint32_t {
    None = (0ul << DB_HASH_SIZE),
    Int8 = (1ul << DB_HASH_SIZE),
    Uint8 = (2ul << DB_HASH_SIZE),
    Int16 = (3ul << DB_HASH_SIZE),
    Uint16 = (4ul << DB_HASH_SIZE),
    Int32 = (5ul << DB_HASH_SIZE),
    Uint32 = (6ul << DB_HASH_SIZE),
    Int64 = (7ul << DB_HASH_SIZE),
    Uint64 = (8ul << DB_HASH_SIZE),
    Float = (9ul << DB_HASH_SIZE),
    String = (10ul << DB_HASH_SIZE),
    Bin = (11ul << DB_HASH_SIZE),
};

class Converter {
   public:
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
            case Type::Int8:
            case Type::Uint8:
                return 1;

            case Type::Int16:
            case Type::Uint16:
                return 2;

            case Type::Int32:
            case Type::Uint32:
                return 4;

            case Type::Int64:
            case Type::Uint64:
                return 8;

            case Type::Float:
                return 4;

            default:
                break;
        }
        return 0;
    }

    String toString() const {
        if (!p) return String();
        switch (type) {
            case Type::Int8:
            case Type::Int16:
            case Type::Int32:
            case Type::Int64:
                return String(toInt32());

            case Type::Uint8:
            case Type::Uint16:
            case Type::Uint32:
            case Type::Uint64:
                return String((uint32_t)toInt32());

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
            case Type::String:
                return (*(char*)p == 't' || *(char*)p == '1');

            default:
                break;
        }
        return toInt16();
    }

    int16_t toInt16() const {
        if (!p) return 0;
        switch (type) {
            case Type::Int8:
            case Type::Uint8:
                return *((int8_t*)p);

            case Type::Int16:
            case Type::Uint16:
                return *((int16_t*)p);

            default:
                break;
        }
        return toInt32();
    }

    int32_t toInt32() const {
        if (!p) return 0;
        switch (type) {
            case Type::Int8:
            case Type::Uint8:
            case Type::Int16:
            case Type::Uint16:
                return toInt16();

            case Type::Int32:
            case Type::Uint32:
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
        return toInt32();
    }

    float toFloat() const {
        if (!p) return 0;
        switch (type) {
            case Type::Float:
                return *((float*)p);

#ifndef DB_NO_FLOAT
            case Type::String:
                return Text((const char*)p, len).toFloat();
#endif
            default:
                break;
        }
        return toInt32();
    }

    Value toValue() const {
        if (!p) return Value();
        switch (type) {
            case Type::Int8:
            case Type::Int16:
                return (int16_t)toInt16();

            case Type::Uint8:
            case Type::Uint16:
                return (uint16_t)toInt16();

            case Type::Int32:
                return (int32_t)toInt32();

            case Type::Uint32:
                return (uint32_t)toInt32();
#ifndef DB_NO_INT64
            case Type::Int64:
                return *((int64_t*)p);
            case Type::Uint64:
                return *((uint64_t*)p);
#endif
#ifndef DB_NO_FLOAT
            case Type::Float:
                return *((float*)p);
#endif
            case Type::String:
                return Text((const char*)p, len);

            default:
                break;
        }
        return Value();
    }

   private:
    Type type;
    const void* p;
    size_t len;
};

}  // namespace gdb