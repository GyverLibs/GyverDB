#pragma once
#include <Arduino.h>
#include <GTL.h>
#include <StringUtils.h>

#include "utils/access.h"
#include "utils/block.h"
#include "utils/entry.h"
#include "utils/utils.h"

// #define DB_NO_UPDATES  // убрать стек обновлений
// #define DB_NO_FLOAT    // убрать поддержку float
// #define DB_NO_INT64    // убрать поддержку int64
// #define DB_NO_CONVERT  // не конвертировать данные (принудительно менять тип записи, keepTypes не работает)

class GyverDB : private gtl::stack_uniq<gdb::block_t> {
   public:
    using gtl::stack_uniq<gdb::block_t>::length;
    using gtl::stack_uniq<gdb::block_t>::capacity;
    using gtl::stack_uniq<gdb::block_t>::reserve;
    using gtl::stack_uniq<gdb::block_t>::valid;
    using gtl::stack_uniq<gdb::block_t>::operator bool;

    GyverDB(uint16_t reserve = 0) {
        if (reserve) this->reserve(reserve);
    }

    ~GyverDB() {
        clear();
    }

    gdb::Access operator[](size_t hash) {
        return gdb::Access(get(hash), hash, this, setHook);
    }
    gdb::Access operator[](const Text& key) {
        return (*this)[key.hash()];
    }

    // не изменять тип записи (конвертировать данные если тип отличается) (умолч. true)
    void keepTypes(bool keep) {
        _keepTypes = keep;
    }

    // использовать стек обновлений (умолч. false)
    void useUpdates(bool use) {
        _useUpdates = use;
    }

    // вывести всё содержимое БД
    void dump(Print& p) {
        p.print(F("DB dump: "));
        p.print(length());
        p.print(F(" entries ("));
        p.print(size());
        p.println(F(" bytes)"));
        for (size_t i = 0; i < length(); i++) {
            if (i <= 9) p.print(0);
            p.print(i);
            p.print(F(". 0x"));
            p.print(_buf[i].keyHash(), HEX);
            p.print(F(" ["));
            p.print(_buf[i].typeRead());
            p.print(F("]: "));
            p.println(gdb::Entry(_buf[i]));
        }
    }

    // экспортный размер БД (для writeTo)
    size_t writeSize() {
        size_t sz = 0;
        for (size_t i = 0; i < length(); i++) {
            if (!_buf[i].valid()) continue;
            if (_buf[i].isDynamic()) {
                if (!_buf[i].ptr()) continue;
                sz += 4 + 2;  // typehash + size
                sz += _buf[i].size();
            } else {
                sz += 4 + 4;  // typehash + data
            }
        }
        sz += 2;  // len
        return sz;
    }

    // экспортировать БД в Stream (напр. файл)
    bool writeTo(Stream& stream) {
        return writeTo(gdb::Writer(stream));
    }

    // экспортировать БД в буфер размера writeSize()
    bool writeTo(uint8_t* buffer) {
        return writeTo(gdb::Writer(buffer));
    }

    // импортировать БД из Stream (напр. файл)
    bool readFrom(Stream& stream, size_t len) {
        return readFrom(gdb::Reader(stream, len));
    }

    // импортировать БД из буфера
    bool readFrom(const uint8_t* buffer, size_t len) {
        return readFrom(gdb::Reader(buffer, len));
    }

    // создать запись. Если существует - перезаписать пустой с новым типом
    bool create(size_t hash, gdb::Type type, uint16_t reserve = 0) {
        pos_t pos = _search(hash);
        if (!pos.exists) {
            gdb::block_t block(type, hash);
            if (!block.init(reserve)) return 0;
            if (insert(pos.idx, block)) {
                _changed = 1;
                return 1;
            } else {
                block.reset();
            }
        } else {
            _setChanged(hash);
            _buf[pos.idx].updateType(type);
            return _buf[pos.idx].init(reserve);
        }
        return 0;
    }
    bool create(const Text& key, gdb::Type type, uint16_t reserve = 0) {
        return create(key.hash(), type, reserve);
    }

    // полностью освободить память
    void reset() {
        clear();
        gtl::stack_uniq<gdb::block_t>::reset();
    }

    // стереть все записи (не освобождает зарезервированное место)
    void clear() {
        while (length()) pop().reset();
        _changed = 1;
    }

    // было изменение данных. После срабатывания сбросится в false
    bool changed() {
        return _changed ? _changed = 0, true : false;
    }

    // полный вес БД
    size_t size() {
        size_t sz = gtl::stack_uniq<gdb::block_t>::size();
        for (size_t i = 0; i < _len; i++) {
            sz += _buf[i].size();
            if (_buf[i].type() == gdb::Type::String) sz++;
        }
        return sz;
    }

    // получить запись
    gdb::Entry get(size_t hash) {
        pos_t pos = _search(hash);
        return (pos.exists) ? gdb::Entry(_buf[pos.idx]) : gdb::Entry();
    }
    gdb::Entry get(const Text& key) {
        return get(key.hash());
    }

    // удалить запись
    void remove(size_t hash) {
        pos_t pos = _search(hash);
        if (pos.exists) {
            _buf[pos.idx].reset();
            gtl::stack_uniq<gdb::block_t>::remove(pos.idx);
            _changed = 1;
        }
    }
    void remove(const Text& key) {
        remove(key.hash());
    }

    // БД содержит запись с именем
    bool has(size_t hash) {
        return _search(hash).exists;
    }
    bool has(const Text& key) {
        return has(key.hash());
    }

    // ================== SET ==================
    bool set(size_t hash, const GyverDB& value) = delete;
    bool set(const Text& key, const GyverDB& value) = delete;

    // bin
    bool set(size_t hash, const void* value, size_t len) { return _set(gdb::Type::Bin, hash, value, len); }
    bool set(const Text& key, const void* value, size_t len) { return set(key.hash(), value, len); }
    template <typename T>
    bool set(size_t hash, const T& value) { return set(hash, &value, sizeof(T)); }
    template <typename T>
    bool set(const Text& key, const T& value) { return set<T>(key.hash(), value); }

    // float
    bool set(size_t hash, float value) { return _set(gdb::Type::Float, hash, &value, 4); }
    bool set(const Text& key, float value) { return set(key.hash(), value); }
    bool set(size_t hash, double value) { return set(hash, (float)value); }
    bool set(const Text& key, double value) { return set(key.hash(), value); }

    // string
    bool set(size_t hash, const Text& value) { return _set(gdb::Type::String, hash, value.str(), value.length()); }
    bool set(const Text& key, const Text& value) { return set(key.hash(), value); }
    bool set(size_t hash, const char* value) { return set(hash, Text(value)); }
    bool set(const Text& key, const char* value) { return set(key.hash(), value); }
    bool set(size_t hash, const String& value) { return set(hash, Text(value)); }
    bool set(const Text& key, const String& value) { return set(key.hash(), value); }

    // int
#define DB_MAKE_INT(type)                                              \
    bool set(size_t hash, type value) { return _setInt(hash, value); } \
    bool set(const Text& key, type value) { return _setInt(key.hash(), value); }

#define DB_MAKE_UINT(type)                                              \
    bool set(size_t hash, type value) { return _setUint(hash, value); } \
    bool set(const Text& key, type value) { return _setUint(key.hash(), value); }

    DB_MAKE_UINT(bool)
    DB_MAKE_UINT(char)
    DB_MAKE_INT(signed char)
    DB_MAKE_UINT(unsigned char)
    DB_MAKE_INT(short)
    DB_MAKE_UINT(unsigned short)
    DB_MAKE_INT(int)
    DB_MAKE_UINT(unsigned int)
    DB_MAKE_INT(long)
    DB_MAKE_UINT(unsigned long)

    bool set(size_t hash, long long value) { return _set(gdb::Type::Int64, hash, &value, 8); }
    bool set(const Text& key, long long value) { return set(key.hash(), value); }
    bool set(size_t hash, unsigned long long value) { return _set(gdb::Type::Uint64, hash, &value, 8); }
    bool set(const Text& key, unsigned long long value) { return set(key.hash(), value); }

    // ===================== INIT =====================
    bool init(size_t hash, const GyverDB& value) = delete;
    bool init(const Text& key, const GyverDB& value) = delete;

    // bin
    bool init(size_t hash, const void* value, size_t len) { return _set(gdb::Type::Bin, hash, value, len, true); }
    bool init(const Text& key, const void* value, size_t len) { return init(key.hash(), value, len); }
    template <typename T>
    bool init(size_t hash, const T& value) { return init(hash, &value, sizeof(T)); }
    template <typename T>
    bool init(const Text& key, const T& value) { return init<T>(key.hash(), value); }

    // float
    bool init(size_t hash, float value) { return _set(gdb::Type::Float, hash, &value, 4, true); }
    bool init(const Text& key, float value) { return init(key.hash(), value); }
    bool init(size_t hash, double value) { return init(hash, (float)value); }
    bool init(const Text& key, double value) { return init(key.hash(), value); }

    // string
    bool init(size_t hash, const Text& value) { return _set(gdb::Type::String, hash, value.str(), value.length(), true); }
    bool init(const Text& key, const Text& value) { return init(key.hash(), value); }
    bool init(size_t hash, const char* value) { return init(hash, Text(value)); }
    bool init(const Text& key, const char* value) { return init(key.hash(), value); }
    bool init(size_t hash, const String& value) { return init(hash, Text(value)); }
    bool init(const Text& key, const String& value) { return init(key.hash(), value); }

    // int
#define DB_MAKE_INIT_INT(type)                                                \
    bool init(size_t hash, type value) { return _setInt(hash, value, true); } \
    bool init(const Text& key, type value) { return _setInt(key.hash(), value, true); }

#define DB_MAKE_INIT_UINT(type)                                                \
    bool init(size_t hash, type value) { return _setUint(hash, value, true); } \
    bool init(const Text& key, type value) { return _setUint(key.hash(), value, true); }

    DB_MAKE_INIT_UINT(bool)
    DB_MAKE_INIT_UINT(char)
    DB_MAKE_INIT_INT(signed char)
    DB_MAKE_INIT_UINT(unsigned char)
    DB_MAKE_INIT_INT(short)
    DB_MAKE_INIT_UINT(unsigned short)
    DB_MAKE_INIT_INT(int)
    DB_MAKE_INIT_UINT(unsigned int)
    DB_MAKE_INIT_INT(long)
    DB_MAKE_INIT_UINT(unsigned long)

    bool init(size_t hash, long long value) { return _set(gdb::Type::Int64, hash, &value, 8, true); }
    bool init(const Text& key, long long value) { return init(key.hash(), value); }
    bool init(size_t hash, unsigned long long value) { return _set(gdb::Type::Uint64, hash, &value, 8, true); }
    bool init(const Text& key, unsigned long long value) { return init(key.hash(), value); }

    // move
    GyverDB(GyverDB& gdb) {
        move(gdb);
    }
    GyverDB& operator=(GyverDB& gdb) {
        move(gdb);
        return *this;
    }

#if __cplusplus >= 201103L
    GyverDB(GyverDB&& gdb) noexcept {
        move(gdb);
    }
    GyverDB& operator=(GyverDB&& gdb) noexcept {
        move(gdb);
        return *this;
    }
#endif

    // есть непрочитанные изменения
    bool updatesAvailable() {
#ifndef DB_NO_UPDATES
        return _updates.length();
#endif
        return 0;
    }

    // получить хеш обновления из стека
    size_t updateNext() {
#ifndef DB_NO_UPDATES
        return _updates.pop();
#endif
        return 0;
    }

    virtual bool tick() { return 0; }

    // hook
    static bool setHook(void* self, gdb::Type type, size_t hash, const void* value, size_t len) {
        return ((GyverDB*)self)->_set(type, hash, value, len);
    }

   protected:
    void move(GyverDB& gdb) noexcept {
        if (this == &gdb) return;
        reset();
        gtl::stack_uniq<gdb::block_t>::move(gdb);
#ifndef DB_NO_UPDATES
        _updates.move(gdb._updates);
#endif
        _keepTypes = gdb._keepTypes;
        _useUpdates = gdb._useUpdates;
        _changed = 1;
    }

   private:
    bool _keepTypes = true;
    bool _changed = false;
    bool _useUpdates = false;

#ifndef DB_NO_UPDATES
    gtl::stack_uniq<size_t> _updates;
    void _setChanged(size_t hash) {
        _changed = true;
        if (_useUpdates && _updates.indexOf(hash) < 0) _updates.push(hash);
    }
#else
    void _setChanged(size_t hash) {
        _changed = true;
    }
#endif

    struct pos_t {
        int idx;
        bool exists;
    };

    pos_t _search(size_t hash) {
        if (!length()) return pos_t{0, false};
        hash &= DB_HASH_MASK;  // to 29bit
        int low = 0, high = length() - 1;
        while (low <= high) {
            int mid = low + ((high - low) >> 1);
            if (_buf[mid].keyHash() == hash) return pos_t{mid, true};
            if (_buf[mid].keyHash() < hash) low = mid + 1;
            else high = mid - 1;
        }
        return pos_t{low, false};
    }

    bool writeTo(gdb::Writer writer) {
        // [bd len] [hash32, value32] [hash32, size16, data...]
        uint16_t len = length();
        writer.write2(len);

        for (size_t i = 0; i < length(); i++) {
            if (!_buf[i].valid()) continue;
            if (_buf[i].isDynamic()) {
                if (!_buf[i].ptr()) continue;
                writer.write4(_buf[i].typehash);
                uint16_t size = _buf[i].size();
                writer.write2(size);
                writer.write(_buf[i].buffer(), size);
            } else {
                writer.write4(_buf[i].typehash);
                writer.write4(_buf[i].data);
            }
        }
        return writer.writed() == writeSize();
    }

    bool readFrom(gdb::Reader reader) {
        clear();
        uint16_t len = 0;
        if (!reader.read2(len)) return 0;
        reserve(len);

        while (reader.available()) {
            uint32_t typehash;
            if (!reader.read4(typehash)) return 0;

            gdb::block_t block;
            block.typehash = typehash;
            if (block.isDynamic()) {
                uint16_t size;
                if (!reader.read2(size)) return 0;
                if (!block.reserve(size)) return 0;
                if (!reader.read(block.buffer(), size)) {
                    block.reset();
                    return 0;
                }
                block.setSize(size);
            } else {
                if (!reader.read4(block.data)) return 0;
            }

            if (!push(block)) {
                block.reset();
                return 0;
            }
            // _changed = 1;
        }
        return 1;
    }

    bool _setInt(size_t hash, int32_t value, bool noupd = false) {
        return _set(gdb::Type::Int, hash, &value, 4, noupd);
    }
    bool _setUint(size_t hash, uint32_t value, bool noupd = false) {
        return _set(gdb::Type::Uint, hash, &value, 4, noupd);
    }

    bool _set(gdb::Type type, size_t hash, const void* value, size_t len, bool noupdate = false) {
        pos_t pos = _search(hash);
        if (pos.exists) {
            if (noupdate) return 1;
            if (_buf[pos.idx].update(type, value, len, _keepTypes)) {
                _setChanged(hash);
                return 1;
            }
        } else {
            gdb::block_t block(type, hash);
            if (block.write(value, len)) {
                if (insert(pos.idx, block)) {
                    _changed = 1;
                    return 1;
                } else {
                    block.reset();
                }
            }
        }
        return 0;
    }
};