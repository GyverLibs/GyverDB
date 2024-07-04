#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "entry.h"
#include "types.h"

namespace gdb {

typedef bool (*setHook)(void* db, gdb::Type type, size_t hash, const void* value, size_t len);

class Access : public Entry {
   public:
    Access(Entry entry, size_t hash, void* db, setHook hook) : Entry(entry), _hash(hash), _db(db), _hook(hook) {}

// int
#define DB_MAKE_ASSIGN_INT(type) \
    bool operator=(type value) { return _int(value); }

#define DB_MAKE_ASSIGN_UINT(type) \
    bool operator=(type value) { return _uint(value); }

    DB_MAKE_ASSIGN_UINT(bool);
    DB_MAKE_ASSIGN_UINT(char);
    DB_MAKE_ASSIGN_INT(signed char);
    DB_MAKE_ASSIGN_UINT(unsigned char);
    DB_MAKE_ASSIGN_INT(short);
    DB_MAKE_ASSIGN_UINT(unsigned short);
    DB_MAKE_ASSIGN_INT(int);
    DB_MAKE_ASSIGN_UINT(unsigned int);
    DB_MAKE_ASSIGN_INT(long);
    DB_MAKE_ASSIGN_UINT(unsigned long);

#ifndef DB_NO_INT64
    bool operator=(long long value) {
        return _hook(_db, gdb::Type::Int64, _hash, &value, 8);
    }
    bool operator=(unsigned long long value) {
        return _hook(_db, gdb::Type::Uint64, _hash, &value, 8);
    }
#endif

    // float
    bool operator=(float value) {
        return _hook(_db, gdb::Type::Float, _hash, &value, 4);
    }
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

    bool _int(int32_t v) {
        return _hook(_db, gdb::Type::Int, _hash, &v, 4);
    }
    bool _uint(uint32_t v) {
        return _hook(_db, gdb::Type::Uint, _hash, &v, 4);
    }
};

}  // namespace gdb