#pragma once
#include <Arduino.h>

#include "block.h"

namespace gdb {

class Entry : protected block_t, public Printable, /*public Text, */ public Converter {
   public:
    using block_t::buffer;
    using block_t::isValidString;
    using block_t::keyHash;
    using block_t::reserve;
    using block_t::size;
    using block_t::type;

    Entry() {}
    Entry(const block_t& b) : block_t(b), /*Text(b.isValidString() ? (const char*)b.buffer() : nullptr, b.size()),*/ Converter(b.type(), b.buffer(), b.size()) {}

    // тип записи
    gdb::Type type() const {
        return block_t::type();
    }

    bool valid() const {
        return block_t::valid();
    }

    size_t printTo(Print& p) const {  // override
        if (type() == gdb::Type::Bin) {
            size_t ret = 0;
            for (size_t b = 0; b < size(); b++) {
                uint8_t data = ((uint8_t*)buffer())[b];
                if (data <= 0xF) ret += p.print('0');
                ret += p.print(data, HEX);
                ret += p.print(' ');
            }
            return ret;
        } else {
            return toText().printTo(p);
        }
    }

    // ======================= EXPORT =======================

    // вывести данные в буфер размера size(). Не добавляет 0-терминатор, если это строка
    void writeBytes(void* buf) const {
        if (block_t::valid() && buffer()) memcpy(buf, buffer(), size());
    }

    // вывести в переменную
    template <typename T>
    bool writeTo(T& dest) const {
        if (block_t::valid() && buffer() && sizeof(T) == size()) {
            writeBytes(&dest);
            return true;
        }
        return false;
    }

    // ======================= CAST =======================

#define DB_MAKE_OPERATOR(T, func)      \
    operator T() const {               \
        return (T)func();              \
    }                                  \
    bool operator==(const T v) const { \
        return (T)func() == v;         \
    }                                  \
    bool operator!=(const T v) const { \
        return (T)func() != v;         \
    }                                  \
    bool operator>(const T v) const {  \
        return (T)func() > v;          \
    }                                  \
    bool operator<(const T v) const {  \
        return (T)func() < v;          \
    }                                  \
    bool operator>=(const T v) const { \
        return (T)func() >= v;         \
    }                                  \
    bool operator<=(const T v) const { \
        return (T)func() <= v;         \
    }

    operator bool() const {
        return toBool();
    }
    bool operator==(const bool v) const {
        return Converter::valid() ? Converter::toBool() == v : false;
    }
    bool operator!=(const bool v) const {
        return *this == !v;
    }

    // TEXT

    bool operator==(const Text& s) const {
        return toText() == s;
    }
    bool operator!=(const Text& s) const {
        return toText() != s;
    }
    bool operator==(const char* s) const {
        return toText() == s;
    }
    bool operator!=(const char* s) const {
        return toText() != s;
    }
    bool operator==(const __FlashStringHelper* s) const {
        return toText() == s;
    }
    bool operator!=(const __FlashStringHelper* s) const {
        return toText() != s;
    }
    bool operator==(const String& s) const {
        return toText() == s;
    }
    bool operator!=(const String& s) const {
        return toText() != s;
    }

    explicit operator Text() const {
        return Converter::toText();
    }
    explicit operator Value() const {
        return Converter::toText();
    }
    operator String() const {
        return Converter::toString();
    }

    // DB_MAKE_OPERATOR(bool, toBool)
    DB_MAKE_OPERATOR(char, toInt)
    DB_MAKE_OPERATOR(signed char, toInt)
    DB_MAKE_OPERATOR(unsigned char, toInt)
    DB_MAKE_OPERATOR(short, toInt)
    DB_MAKE_OPERATOR(unsigned short, toInt)
    DB_MAKE_OPERATOR(int, toInt)
    DB_MAKE_OPERATOR(unsigned int, toInt)
    DB_MAKE_OPERATOR(long, toInt)
    DB_MAKE_OPERATOR(unsigned long, toInt)
    DB_MAKE_OPERATOR(long long, toInt64)
    DB_MAKE_OPERATOR(unsigned long long, toInt64)
    DB_MAKE_OPERATOR(float, toFloat)
    DB_MAKE_OPERATOR(double, toFloat)

   private:
};

}  // namespace gdb
