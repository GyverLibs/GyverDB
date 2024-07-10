#pragma once
#include <Arduino.h>
#include <FS.h>

#include "GyverDB.h"

class GyverDBFile : public GyverDB {
   public:
    GyverDBFile(fs::FS* nfs = nullptr, const char* path = nullptr, uint32_t tout = 10000) {
        setFS(nfs, path);
        _tout = tout;
    }

    // установить файловую систему и имя файла
    void setFS(fs::FS* nfs, const char* path) {
        _fs = nfs;
        _path = path;
    }

    // установить таймаут записи, мс (умолч. 10000)
    void setTimeout(uint32_t tout = 10000) {
        _tout = tout;
    }

    // прочитать данные
    bool begin() {
        if (_fs) {
            if (_fs->exists(_path)) {
                File file = _fs->open(_path, "r");
                if (file) return readFrom(file, file.size());
            } else {
                File file = _fs->open(_path, "w");
                return 1;
            }
        }
        return 0;
    }

    // обновить данные в файле
    bool update() {
        _changed = false;
        File file = _fs->open(_path, "w");
        return file ? writeTo(file) : 0;
    }

    // тикер, вызывать в loop. Сам обновит данные при изменении и выходе таймаута, вернёт true
    bool tick() {
        if (changed()) {
            _tmr = millis();
            if (!_tmr) _tmr = 1;
        }
        if (_tmr && millis() - _tmr >= _tout) {
            _tmr = 0;
            update();
            return 1;
        }
        return 0;
    }

   private:
    fs::FS* _fs;
    const char* _path;
    uint32_t _tmr = 0, _tout = 10000;

    using GyverDB::changed;
};