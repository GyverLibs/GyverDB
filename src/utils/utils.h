#pragma once
#include <Arduino.h>

namespace gdb {

class Reader {
   public:
    Reader(Stream& stream, size_t len) : stream(&stream), len(len) {}
    Reader(const uint8_t* buffer, size_t len) : buffer(buffer), len(len) {}

    bool read(void* dest, size_t size) {
        if (size > len) return 0;

        if (stream) {
            len -= size;
            return stream->readBytes((uint8_t*)dest, size) == size;
        } else if (buffer) {
            memcpy(dest, buffer, size);
            len -= size;
            buffer += size;
            return 1;
        }
        return 0;
    }

    inline bool read4(uint32_t& val) {
        return read(&val, 4);
    }
    inline bool read2(uint16_t& val) {
        return read(&val, 2);
    }

    bool available() {
        return len;
    }

   private:
    Stream* stream = nullptr;
    const uint8_t* buffer = nullptr;
    size_t len = 0;
};

class Writer {
   public:
    Writer(Stream& stream) : stream(&stream) {}
    Writer(uint8_t* buffer) : buffer(buffer) {}

    void write(const void* data, size_t size) {
        if (stream) {
            _writed += stream->write((const uint8_t*)data, size);
        } else if (buffer) {
            memcpy(buffer, data, size);
            buffer += size;
            _writed += size;
        }
    }

    inline void write4(uint32_t& val) {
        write(&val, 4);
    }
    inline void write2(uint16_t& val) {
        write(&val, 2);
    }

    size_t writed() {
        return _writed;
    }

   private:
    Stream* stream = nullptr;
    uint8_t* buffer = nullptr;
    size_t _writed = 0;
};

}  // namespace gdb