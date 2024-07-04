#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entry.h"
#include "types.h"

#define DB_MAKE_ASSIGN(type, dbtype, size)              \
    bool operator=(type value) {                        \
        return _hook(_db, dbtype, _hash, &value, size); \
    }

namespace gdb {

typedef bool (*setHook)(void* db, gdb::Type type, size_t hash, const void* value, size_t len);

class Access : public Entry {
   public:
    Access(Entry entry, size_t hash, void* db, setHook hook) : Entry(entry), _hash(hash), _db(db), _hook(hook) {}

    // num
    DB_MAKE_ASSIGN(bool, gdb::Type::Uint8, 1);
    DB_MAKE_ASSIGN(char, gdb::Type::Uint8, 1);
    DB_MAKE_ASSIGN(signed char, gdb::Type::Int8, 1);
    DB_MAKE_ASSIGN(unsigned char, gdb::Type::Uint8, 1);

    DB_MAKE_ASSIGN(short, gdb::Type::Int16, 2);
    DB_MAKE_ASSIGN(unsigned short, gdb::Type::Uint16, 2);

#if (UINT_MAX == UINT32_MAX)
    DB_MAKE_ASSIGN(int, gdb::Type::Int32, 4);
    DB_MAKE_ASSIGN(unsigned int, gdb::Type::Uint32, 4);
#else
    DB_MAKE_ASSIGN(int, gdb::Type::Int16, 2);
    DB_MAKE_ASSIGN(unsigned int, gdb::Type::Uint16, 2);
#endif

    DB_MAKE_ASSIGN(long, gdb::Type::Int32, 4);
    DB_MAKE_ASSIGN(unsigned long, gdb::Type::Uint32, 4);

#ifndef DB_NO_INT64
    DB_MAKE_ASSIGN(long long, gdb::Type::Int64, 8);
    DB_MAKE_ASSIGN(unsigned long long, gdb::Type::Uint64, 8);
#endif

    DB_MAKE_ASSIGN(float, gdb::Type::Float, 4);
    bool operator=(double value) {
        return *this = (float)value;
    }

    // bin
    template <typename T>
    bool operator=(const T& value) {
        return _hook(_db, gdb::Type::Bin, _hash, &value, sizeof(T));
    }

    // string
    bool operator=(const Text& value) {
        return _hook(_db, gdb::Type::String, _hash, value.str(), value.length());
    }
    bool operator=(const String& value) {
        return _hook(_db, gdb::Type::String, _hash, value.c_str(), value.length());
    }
    bool operator=(const char* value) {
        return _hook(_db, gdb::Type::String, _hash, value, strlen(value));
    }

   private:
    size_t _hash;
    void* _db;
    setHook _hook;
};

}  // namespace gdb