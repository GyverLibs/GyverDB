// простой пример настройки подключения к WiFi с DBConnector

#include <Arduino.h>
#include <LittleFS.h>
#include <DBConnector.h>

DB_KEYS(
    kk,
    DB_KEY(wifi_ssid),
    DB_KEY(wifi_pass)
);

GyverDBFile db(&LittleFS, "/data.db");
DBConnector wifi(&db, kk::wifi_ssid, kk::wifi_pass);

void setup() {
    Serial.begin(115200);
    Serial.println();

    // базу данных запускаем до подключения к точке
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif
    db.begin();
    db[kk::wifi_ssid] = "SSID";
    db[kk::wifi_pass] = "PASS";

    // подключение и реакция на подключение или ошибку
    wifi.onConnect([]() {
        Serial.print("Connected! ");
        Serial.println(WiFi.localIP());
    });
    wifi.onError([]() {
        Serial.print("Error! start AP ");
        Serial.println(WiFi.softAPIP());
    });
    wifi.connect();
}

void loop() {
    wifi.tick();
}