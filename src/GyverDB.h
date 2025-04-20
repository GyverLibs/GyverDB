#pragma once
#include <Arduino.h>
#include <StreamIO.h>
#include <StringUtils.h>
#include <GTL.h>

#include "utils/access.h"
#include "utils/anytype.h"
#include "utils/block.h"
#include "utils/entry.h"

// #define DB_NO_UPDATES  // убрать стек обновлений
// #define DB_NO_FLOAT    // убрать поддержку float
// #define DB_NO_INT64    // убрать поддержку int64
// #define DB_NO_CONVERT  // не конвертировать данные (принудительно менять тип ячейки, keepTypes не работает)

class GyverDB : private gtl::stack<gdb::block_t> {
    typedef gtl::stack<gdb::block_t> ST;

    enum class Putmode {
        Set,
        Init,
        Update,
    };

   public:
    using ST::capacity;
    using ST::length;
    using ST::reserve;
    using ST::stack;
    using ST::valid;
    using ST::operator bool;

    GyverDB(uint16_t reserveEntries = 0) {
        reserve(reserveEntries);
    }
    GyverDB(const GyverDB& db) = delete;
    GyverDB(GyverDB& db) {
        move(db);
    }
    GyverDB(GyverDB&& db) noexcept {
        move(db);
    }
    GyverDB& operator=(GyverDB db) {
        move(db);
        return *this;
    }
    
    void move(GyverDB& db) noexcept {
        ST::move(db);
#ifndef DB_NO_UPDATES
        _updates.move(db._updates);
#endif
        gtl::swap(_keepTypes, db._keepTypes);
        gtl::swap(_useUpdates, db._useUpdates);
        gtl::swap(_cache, db._cache);
        gtl::swap(_cache_h, db._cache_h);
        _change();
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

    // не изменять тип ячейки (конвертировать данные если тип отличается) (умолч. true)
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
    template <typename T>
    bool writeTo(T& writer) {
        // [db len] [hash32, value32] [hash32, size16, data...]

#define _DB_WRITE(x) wr += writer.write((uint8_t*)&x, sizeof(x))

        size_t wr = 0;
        uint16_t len = length();
        _DB_WRITE(len);
        for (size_t i = 0; i < length(); i++) {
            if (!_buf[i].valid()) continue;
            if (_buf[i].isDynamic()) {
                if (!_buf[i].ptr()) continue;
                _DB_WRITE(_buf[i].typehash);
                uint16_t size = _buf[i].size();
                _DB_WRITE(size);
                wr += writer.write((uint8_t*)_buf[i].buffer(), size);
            } else {
                _DB_WRITE(_buf[i].typehash);
                _DB_WRITE(_buf[i].data);
            }
        }
        return wr == writeSize();
    }

    // экспортировать БД в буфер размера writeSize()
    bool writeTo(uint8_t* buffer) {
        Writer wr(buffer);
        return writeTo(wr);
    }

    // импортировать БД из Stream (напр. файл)
    bool readFrom(Stream& stream, size_t len) {
        return readFrom(Reader(stream, len));
    }

    // импортировать БД из буфера
    bool readFrom(const uint8_t* buffer, size_t len) {
        return readFrom(Reader(buffer, len));
    }

    // создать ячейку. Если существует - перезаписать пустой с новым типом
    bool create(size_t hash, gdb::Type type, uint16_t reserve = 0) {
        pos_t pos = _search(hash);
        if (!pos.exists) {
            _cache = -1;
            gdb::block_t block(type, hash);
            if (!block.init(reserve)) return 0;
            if (insert(pos.idx, block)) {
                _change();
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
        ST::reset();
    }

    // стереть все ячейки (не освобождает зарезервированное место)
    void clear() {
        _cache = -1;
        while (length()) pop().reset();
        _change();
    }

    // удалить из БД ячейки, ключей которых нет в переданном списке
    void cleanup(size_t* hashes, size_t len) {
        _cache = -1;
        for (size_t i = 0; i < _len;) {
            size_t hash = _buf[i].keyHash();
            bool found = false;
            for (size_t h = 0; h < len; h++) {
                if (hash == (hashes[h] & DB_HASH_MASK)) {
                    i++;
                    found = true;
                    break;
                }
            }
            if (!found) {
                ST::remove(i);
                _change();
            }
        }
    }

    // вывести все ключи в массив длиной length()
    void getKeys(size_t* hashes) {
        for (size_t i = 0; i < _len; i++) {
            hashes[i] = _buf[i].keyHash();
        }
    }

    // было изменение бд
    bool changed() {
        return _changed;
    }

    // сбросить флаг изменения бд
    void clearChanged() {
        _changed = false;
    }

    // полный вес БД
    size_t size() {
        size_t sz = ST::size();
        for (size_t i = 0; i < _len; i++) {
            sz += _buf[i].size();
            if (_buf[i].type() == gdb::Type::String) sz++;
        }
        return sz;
    }

    // получить ячейку
    gdb::Entry get(size_t hash) {
        if (~_cache && _cache_h == hash) return gdb::Entry(_buf[_cache]);
        else {
            pos_t pos = _search(hash);
            if (pos.exists) {
                _cache = pos.idx;
                _cache_h = hash;
                return gdb::Entry(_buf[_cache]);
            }
        }
        return gdb::Entry();
    }
    gdb::Entry get(const Text& key) {
        return get(key.hash());
    }

    // получить ячейку по порядку
    gdb::Entry getN(int idx) {
        return (idx < (int)_len) ? gdb::Entry(_buf[idx]) : gdb::Entry();
    }

    // удалить ячейку
    void remove(size_t hash) {
        pos_t pos = _search(hash);
        if (pos.exists) {
            _cache = -1;
            _buf[pos.idx].reset();
            ST::remove(pos.idx);
            _change();
        }
    }
    void remove(const Text& key) {
        remove(key.hash());
    }

    // БД содержит ячейку с именем
    bool has(size_t hash) {
        return _search(hash).exists;
    }
    bool has(const Text& key) {
        return has(key.hash());
    }

    // ================== SET ==================
    bool set(size_t hash, gdb::AnyType val) { return _put(hash, val, Putmode::Set); }
    bool set(const Text& key, gdb::AnyType val) { return _put(key.hash(), val, Putmode::Set); }

    // ================== INIT ==================
    bool init(size_t hash, gdb::AnyType val) { return _put(hash, val, Putmode::Init); }
    bool init(const Text& key, gdb::AnyType val) { return _put(key.hash(), val, Putmode::Init); }

    // ================== UPDATE ==================
    bool update(size_t hash, gdb::AnyType val) { return _put(hash, val, Putmode::Update); }
    bool update(const Text& key, gdb::AnyType val) { return _put(key.hash(), val, Putmode::Update); }

    // ===================== MISC =====================
    // есть непрочитанные изменения
    bool updatesAvailable() {
#ifndef DB_NO_UPDATES
        return _updates.length();
#endif
        return 0;
    }

    // пропустить необработанные обновления
    void skipUpdates() {
#ifndef DB_NO_UPDATES
        _updates.clear();
#endif
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
    static bool setHook(void* db, size_t hash, const gdb::AnyType& val) {
        return ((GyverDB*)db)->_put(hash, val, Putmode::Set);
    }

   protected:
    bool _update = 0;

   private:
    bool _keepTypes = true;
    bool _useUpdates = false;
    bool _changed = false;
    int16_t _cache = -1;
    size_t _cache_h = 0;

#ifndef DB_NO_UPDATES
    gtl::stack<size_t> _updates;
#endif

    void _setChanged(size_t hash) {
        _change();
#ifndef DB_NO_UPDATES
        if (_useUpdates && _updates.indexOf(hash) < 0) _updates.push(hash);
#endif
    }

    struct pos_t {
        int idx;
        bool exists;
    };

    void _change() {
        _changed = true;
        _update = true;
    }

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

    bool readFrom(Reader reader) {
        clear();
        uint16_t len = 0;
        if (!reader.read(len)) return 0;
        reserve(len);

        while (reader.available()) {
            uint32_t typehash;
            if (!reader.read(typehash)) return 0;

            gdb::block_t block;
            block.typehash = typehash;
            if (block.isDynamic()) {
                uint16_t size;
                if (!reader.read(size)) return 0;
                if (!block.reserve(size)) return 0;
                if (!reader.read(block.buffer(), size)) {
                    block.reset();
                    return 0;
                }
                block.setSize(size);
            } else {
                if (!reader.read(block.data)) return 0;
            }

            if (!push(block)) {
                block.reset();
                return 0;
            }
        }
        _change();
        return 1;
    }

    bool _put(size_t hash, const gdb::AnyType& val, Putmode mode) {
        pos_t pos = _search(hash);
        if (pos.exists) {
            if (mode == Putmode::Init && _buf[pos.idx].type() == val.type) return 0;

            if (_buf[pos.idx].update(val.type, val.ptr, val.len, (_keepTypes && mode != Putmode::Init))) {
                _setChanged(hash);
                return 1;
            }
        } else {
            _cache = -1;
            if (mode == Putmode::Update) return 0;

            gdb::block_t block(val.type, hash);
            if (block.write(val.ptr, val.len)) {
                if (insert(pos.idx, block)) {
                    _change();
                    return 1;
                } else {
                    block.reset();
                }
            }
        }
        return 0;
    }
};
