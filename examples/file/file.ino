#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>

GyverDBFile db(&LittleFS, "/data.db");

// имена ячеек базы данных
DB_KEYS(
    kk,
    input,
    val12
);

void setup() {
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif

    // запуск и инициализация полей БД
    db.begin();
    db.init(kk::input, 0);
    db.init(kk::val12, "text");
}

void loop() {
    // тикер, вызывать в лупе
    db.tick();
}