#include <Arduino.h>
#include <GyverDB.h>

GyverDB db;

DB_KEYS(keys,
    key1,
    arr  // последнюю запятую не ставим
);

void setup() {
    Serial.begin(115200);

    db["bool"_h] = true;       // хеш _h
    db[keys::key1] = 1234;     // хеш из enum
    db[SH("intval")] = -12321; // SH(хеш)
    db["uintval"] = 12345;     // просто строка
    db["u64"] = 123456789876ULL;
    db["i64"] = -12345678987654321LL;
    db["fl"] = 3.14;
    db["str"] = "abcdefg";
    uint8_t arr[5] = {0x09, 0x0F, 0x10, 0xa, 0xaa};
    db[keys::arr] = arr;

    db.dump(Serial);
    
    int i = db["intval"]; // авто конверсия
    i = db["intval"].toBool(); // ручная 
    db["bool"_h] == true;
    i = db["intval"].toFloat();
    i = db["intval"].toInt();
    Serial.println(db["intval"]); // печатается
}

void loop() {
}
