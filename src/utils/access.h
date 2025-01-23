#pragma once
#include <Arduino.h>

#include "anytype.h"
#include "entry.h"
#include "types.h"

namespace gdb {

typedef bool (*setHook)(void* db, size_t hash, const AnyType& val);

class Access : public Entry {
   public:
    Access(Entry entry, size_t hash, void* db, setHook hook) : Entry(entry), _hash(hash), _db(db), _hook(hook) {}

    bool operator=(AnyType value) {
        return _hook(_db, _hash, value);
    }

    // incr decr
    template <typename T>
    T operator++() {
        T value = (T)(*this) + 1;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator++(T) {
        T value = (T)(*this) + 1;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator--() {
        T value = (T)(*this) - 1;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator--(T) {
        T value = (T)(*this) - 1;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    // compl
    template <typename T>
    T operator+=(T value) {
        value = (T)(*this) + value;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator-=(T value) {
        value = (T)(*this) - value;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator*=(T value) {
        value = (T)(*this) * value;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator/=(T value) {
        value = (T)(*this) / value;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator%=(T value) {
        value = (T)(*this) % value;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    // bin
    template <typename T>
    T operator|=(T value) {
        value = (T)(*this) | value;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

    template <typename T>
    T operator&=(T value) {
        value = (T)(*this) & value;
        _hook(_db, _hash, AnyType(value));
        return value;
    }

   private:
    size_t _hash;
    void* _db;
    setHook _hook;
};

}  // namespace gdb