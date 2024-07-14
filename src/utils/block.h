#pragma once
#include <Arduino.h>

#include "types.h"

namespace gdb {

class block_t {
   public:
    block_t() {}
    block_t(Type type, size_t hash) : typehash(DB_MAKE_TYPEHASH(type, hash)) {}

    uint32_t typehash = 0;
    uint32_t data = 0;

    // указатель на динамические данные
    inline void* ptr() const {
        return (void*)data;
    }

    // указатель непосредственно на данные размера size()
    void* buffer() const {
        switch (type()) {
            case Type::Int:
            case Type::Uint:
            case Type::Float:
                return (void*)&data;

#ifndef DB_NO_INT64
            case Type::Int64:
            case Type::Uint64:
                return ptr();
#endif
            case Type::String:
            case Type::Bin:
                return ptr() ? ((uint8_t*)ptr() + 2) : nullptr;

            default:
                break;
        }
        return nullptr;
    }

    // хэш ключа записи
    inline size_t keyHash() const {
        return DB_GET_HASH(typehash);
    }

    // тип записи
    inline Type type() const {
        return DB_GET_TYPE(typehash);
    }

    // тип записи как pgm строка
    const __FlashStringHelper* typeRead() const {
        switch (type()) {
            case Type::Bin: return F("Bin");
            case Type::Int: return F("Int");
            case Type::Uint: return F("Uint");
            case Type::Int64: return F("Int64");
            case Type::Uint64: return F("Uint64");
            case Type::Float: return F("Float");
            case Type::String: return F("String");
            default: break;
        }
        return F("None");
    }

    // запись валидна
    inline bool valid() const {
        return typehash;
    }

    // запись валидна
    inline operator bool() const {
        return valid();
    }

    // запись является валидной строкой
    bool isValidString() const {
        return (ptr() && type() == Type::String);
    }

    // освободить динамический буфер и сбросить тип
    void reset() {
        if (isDynamic() && ptr()) free(ptr());
        typehash = DB_REPLACE_TYPE(typehash, Type::None);
        data = 0;
    }

    // зарезервировать место в количестве данных. true - buffer() всегда валидный
    bool reserve(size_t len) {
        if (valid()) {
            if (isDynamic()) {
                void* p = realloc(ptr(), realLen(len));
                if (!p) return 0;
                data = (uint32_t)p;
                return 1;
            } else {
                if (len <= 4) return 1;
            }
        }
        return 0;
    }

    // обновить тип
    void updateType(Type newtype) {
        if (isDynamic() != Converter::isDynamic(newtype)) {
            if (isDynamic()) reset();
            else data = 0;
        }
        typehash = DB_REPLACE_TYPE(typehash, newtype);
    }

    // записать данные текущего типа
    bool write(const void* value, size_t len) {
        if (!valid()) return 0;
        if (!reserve(len)) return 0;
        if (!isDynamic()) data = 0;
        memcpy(buffer(), value, len);
        setSize(len);
        return 1;
    }

    // сравнить данные и записать если отличаются
    bool compareAndUpdate(const void* value, size_t len) {
        if (len == size() && !memcmp(value, buffer(), len)) return 0;  // same
        return write(value, len);
    }

    // обновить
    bool update(Type ntype, const void* value, size_t len, bool keepType) {
        if (!valid() || ntype == Type::None) return 0;

        if (type() == ntype) {
            return compareAndUpdate(value, len);  //!!!!!!!!!
        } else {
#ifndef DB_NO_CONVERT
            if (keepType) {
                Converter conv(ntype, value, len);
                switch (type()) {
                    case Type::String: {
                        Value v = conv.toText();
                        return compareAndUpdate(v.str(), v.length());
                    }
                    case Type::Int:
                    case Type::Uint: {
                        uint32_t v = conv.toInt();
                        return compareAndUpdate(&v, 4);
                    }
#ifndef DB_NO_INT64
                    case Type::Int64:
                    case Type::Uint64: {
                        uint64_t v = conv.toInt64();
                        return compareAndUpdate(&v, 8);
                    } break;
#endif
#ifndef DB_NO_FLOAT
                    case Type::Float: {
                        float v = conv.toFloat();
                        return compareAndUpdate(&v, 4);
                    }
#endif
                    default:
                        return 0;
                }
            } else
#endif
            {
                updateType(ntype);
                return write(value, len);
            }
        }
        return 1;
    }

    // записать размер для raw и string
    void setSize(size_t len) {
        switch (type()) {
            case Type::Bin:
            case Type::String:
                if (ptr()) {
                    *((uint16_t*)ptr()) = len;
                    if (type() == Type::String) ((char*)buffer())[len] = 0;
                }
                break;

            default:
                break;
        }
    }

    // создать пустую запись и зарезервировать место
    bool init(uint16_t len = 0) {
        switch (type()) {
            case Type::Bin:
                if (!reserve(len)) return 0;
                setSize(0);
                break;

            case Type::String:
                if (!reserve(len)) return 0;
                setSize(0);
                break;
#ifndef DB_NO_INT64
            case Type::Int64:
            case Type::Uint64:
                if (!reserve(8)) return 0;
                memset(ptr(), 0, 8);
#endif
            default:
                break;
        }
        return 1;
    }

    // скорректировать длину
    size_t realLen(size_t len) const {
        switch (type()) {
            case Type::Bin: return len + 2;
            case Type::String: return len + 2 + 1;
            default: break;
        }
        return len;
    }

    // вес данных / длина строки без 0-терминатора
    size_t size() const {
        switch (type()) {
            case Type::String:
            case Type::Bin:
                return ptr() ? *((uint16_t*)ptr()) : 0;

            default:
                return Converter::size(type());
        }
        return 0;
    }

    // запись хранит данные динамически
    bool isDynamic() const {
        return Converter::isDynamic(type());
    }
};

}  // namespace gdb
