#pragma once
#include "GyverDBFile.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

class DBConnector {
    typedef std::function<void()> ConnectCallback;

   public:
    // подключить БД, указать ключи логина и пароля, имя точки, таймаут в секундах и очистку SSID при выходе таймаута
    DBConnector(GyverDBFile* db,
                size_t ssid,
                size_t pass,
                const char* APname = "ESP AP",
                uint32_t timeout = 60,
                bool resetSSID = false) : _db(db),
                                          _ssid(ssid),
                                          _pass(pass),
                                          _APname(APname),
                                          _tout(timeout * 1000ul),
                                          _resetSSID(resetSSID) {}

    // прдключить обработчик успешного подключения
    void onConnect(ConnectCallback cb) {
        _conn_cb = cb;
    }

    // прдключить обработчик ошибки (запущен AP)
    void onError(ConnectCallback cb) {
        _err_cb = cb;
    }

    // подключиться
    bool connect() {
        _db->init(_ssid, "");
        _db->init(_pass, "");

        if ((*_db)[_ssid].length()) {
            _tryConnect = true;
            _tmr = millis();
            WiFi.softAPdisconnect();
            WiFi.mode(WIFI_STA);
            WiFi.begin((*_db)[_ssid], (*_db)[_pass]);
            return 1;
        } else {
            _startAP();
        }
        return 0;
    }

    // состояние подключения. true - подключен, false - запущена АР
    bool connected() {
        return WiFi.status() == WL_CONNECTED;
    }

    // вызывать в loop. Вернёт true при смене состояния
    bool tick() {
        if (_tryConnect) {
            if (WiFi.status() == WL_CONNECTED) {
                _tryConnect = false;
                if (_conn_cb) _conn_cb();
                return 1;
            } else if (millis() - _tmr >= _tout) {
                _startAP();
                if (_resetSSID) resetSSID();
                return 1;
            }
        }
        return 0;
    }

    // сбросить ssid
    void resetSSID() {
        _db->set(_ssid, "");
        _db->update();
    }

   private:
    GyverDBFile* _db;
    size_t _ssid, _pass;
    const char* _APname;

    bool _tryConnect = false;
    uint32_t _tmr = 0, _tout;
    bool _resetSSID;
    ConnectCallback _conn_cb = nullptr;
    ConnectCallback _err_cb = nullptr;

    void _startAP() {
        _tryConnect = false;
        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        WiFi.softAP(_APname);
        if (_err_cb) _err_cb();
    }
};