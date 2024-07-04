#pragma once
#include <Arduino.h>
#include <StringUtils.h>
#include <limits.h>

#include "block.h"

namespace gdb {

class Entry : protected block_t, public Text {
    using Text::toBool;
    using Text::toFloat;
    using Text::toInt;
    using Text::toInt16;
    using Text::toInt32;
    using Text::toInt64;
    using Text::toString;
    using Text::type;
    using Text::operator bool;
    using Text::operator double;
    using Text::operator float;
    using Text::operator int;
    using Text::operator long long;
    using Text::operator long;
    using Text::operator short;
    using Text::operator signed char;
    using Text::operator unsigned char;
    using Text::operator unsigned long long;
    using Text::operator unsigned long;
    using Text::operator unsigned short;

   public:
    using block_t::buffer;
    using block_t::isValidString;
    using block_t::keyHash;
    using block_t::reserve;
    using block_t::size;
    using block_t::type;
    using block_t::valid;
    using block_t::operator bool;

    Entry() {}
    Entry(const block_t& b) : block_t(b), Text(isValidString() ? (const char*)buffer() : nullptr, size()) {}

    // тип записи
    gdb::Type type() const {
        return block_t::type();
    }

    // ambiguous String()
    // operator const char*() const {
    //     return isValidString() ? (const char*)buffer() : "";
    // }

    bool valid() const {
        return block_t::valid();
    }

    explicit operator bool() const {
        return block_t::valid();
    }

    size_t printTo(Print& p) const {
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

    // String toString() const {
    //     return Converter(type(), buffer(), size()).toString();
    // }

    // override
    bool addString(String& s) const {
        toText().addString(s);
        return 1;
    }

    bool toBool() const {
        return Converter(type(), buffer(), size()).toBool();
    }

    int toInt() const {
#if (UINT_MAX == UINT32_MAX)
        return toInt32();
#else
        return toInt16();
#endif
    }

    int8_t toInt8() const {
        return toInt16();
    }

    int16_t toInt16() const {
        return Converter(type(), buffer(), size()).toInt16();
    }

    int32_t toInt32() const {
        return Converter(type(), buffer(), size()).toInt32();
    }

    int64_t toInt64() const {
        return Converter(type(), buffer(), size()).toInt64();
    }

    double toFloat() const {
        return Converter(type(), buffer(), size()).toFloat();
    }

    // ======================= CAST =======================

    // signed char
    operator signed char() const {
        return (char)toInt16();
    }
    bool operator==(const signed char v) const {
        return toInt16() == v;
    }
    bool operator!=(const signed char v) const {
        return toInt16() != v;
    }

    // unsigned char
    operator unsigned char() const {
        return toInt16();
    }
    bool operator==(const unsigned char v) const {
        return (unsigned char)toInt16() == v;
    }
    bool operator!=(const unsigned char v) const {
        return (unsigned char)toInt16() != v;
    }

    // short
    operator short() const {
        return toInt16();
    }
    bool operator==(const short v) const {
        return toInt16() == v;
    }
    bool operator!=(const short v) const {
        return toInt16() != v;
    }

    // unsigned short
    operator unsigned short() const {
        return toInt16();
    }
    bool operator==(const unsigned short v) const {
        return (unsigned short)toInt16() == v;
    }
    bool operator!=(const unsigned short v) const {
        return (unsigned short)toInt16() != v;
    }

    // int
    operator int() const {
        return toInt();
    }
    bool operator==(const int v) const {
        return toInt() == v;
    }
    bool operator!=(const int v) const {
        return toInt() != v;
    }

    // unsigned int
    operator unsigned int() const {
        return toInt();
    }
    bool operator==(const unsigned int v) const {
        return (unsigned int)toInt() == v;
    }
    bool operator!=(const unsigned int v) const {
        return (unsigned int)toInt() != v;
    }

    // long
    operator long() const {
        return toInt32();
    }
    bool operator==(const long v) const {
        return toInt32() == v;
    }
    bool operator!=(const long v) const {
        return toInt32() != v;
    }

    // unsigned long
    operator unsigned long() const {
        return toInt32();
    }
    bool operator==(const unsigned long v) const {
        return (unsigned long)toInt32() == v;
    }
    bool operator!=(const unsigned long v) const {
        return (unsigned long)toInt32() != v;
    }

    // long long
    operator long long() const {
        return toInt64();
    }
    bool operator==(const long long v) const {
        return toInt64() == v;
    }
    bool operator!=(const long long v) const {
        return toInt64() != v;
    }

    // unsigned long long
    operator unsigned long long() const {
        return toInt64();
    }
    bool operator==(const unsigned long long& v) const {
        return (unsigned long long)toInt64() == v;
    }
    bool operator!=(const unsigned long long& v) const {
        return (unsigned long long)toInt64() != v;
    }

    // float
    operator float() const {
        return toFloat();
    }
    bool operator==(const float v) const {
        return toFloat() == v;
    }
    bool operator!=(const float v) const {
        return toFloat() != v;
    }

    // double
    operator double() const {
        return toFloat();
    }
    bool operator==(const double& v) const {
        return toFloat() == v;
    }
    bool operator!=(const double& v) const {
        return toFloat() != v;
    }
};

}  // namespace gdb
