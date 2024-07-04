#include <Arduino.h>
#include <GyverDB.h>

GyverDB db;

enum keys : size_t {
    key1,
    arr,
};

void setup() {
    Serial.begin(115200);

    db["bool"_h] = true;
    db[keys::key1] = 1234;
    db[SH("i16")] = (int16_t)-12321;
    db["i32"] = (int32_t)-123454321;
    db["u32"] = (uint32_t)0xff0000;
    db["i64"] = (int64_t)-12345678987654321;
    db["fl"] = 3.14f;
    db["str"] = "abcdefg";
    uint8_t arr[5] = {0x09, 0x0F, 0x10, 0xa, 0xaa};
    db[keys::arr] = arr;

    db.dump(Serial);
}

void loop() {
}