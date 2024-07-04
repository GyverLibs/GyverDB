#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "block.h"

namespace gdb {

class Entry : protected block_t, public Text {
   public:
    using block_t::buffer;
    using block_t::isValidString;
    using block_t::keyHash;
    using block_t::reserve;
    using block_t::size;
    using block_t::type;

    Entry() {}
    Entry(const block_t& b) : block_t(b), Text(isValidString() ? (const char*)buffer() : nullptr, size()) {}

    // тип записи
    gdb::Type type() const {
        return block_t::type();
    }

    bool valid() const {
        return block_t::valid();
    }

    explicit operator bool() const {
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
            return 1;
        }
        return 0;
    }

    Value toText() const {
        return Converter(type(), buffer(), size()).toValue();
    }

    bool addString(String& s) const {  // override
        toText().addString(s);
        return 1;
    }

    bool toBool() const {
        return Converter(type(), buffer(), size()).toBool();
    }

    int32_t toInt() const {
        return Converter(type(), buffer(), size()).toInt();
    }

    int64_t toInt64() const {
        return Converter(type(), buffer(), size()).toInt64();
    }

    double toFloat() const {
        return Converter(type(), buffer(), size()).toFloat();
    }

    // ======================= CAST =======================

#define DB_MAKE_OPERATOR(type, func)      \
    operator type() const {               \
        return (type)func();              \
    }                                     \
    bool operator==(const type v) const { \
        return (type)func() == v;         \
    }                                     \
    bool operator!=(const type v) const { \
        return (type)func() != v;         \
    }                                     \
    bool operator>(const type v) const {  \
        return (type)func() > v;          \
    }                                     \
    bool operator<(const type v) const {  \
        return (type)func() < v;          \
    }                                     \
    bool operator>=(const type v) const { \
        return (type)func() >= v;         \
    }                                     \
    bool operator<=(const type v) const { \
        return (type)func() <= v;         \
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
    using Text::toBool;
    using Text::toFloat;
    using Text::toInt;
    using Text::toInt16;
    using Text::toInt32;
    using Text::toInt64;
    using Text::toString;
    using Text::type;
    using Text::operator bool;
    using Text::operator char;
    using Text::operator signed char;
    using Text::operator short;
    using Text::operator unsigned short;
    using Text::operator int;
    using Text::operator unsigned int;
    using Text::operator long;
    using Text::operator unsigned long;
    using Text::operator long long;
    using Text::operator unsigned long long;
    using Text::operator float;
    using Text::operator double;
};

}  // namespace gdb
