[![latest](https://img.shields.io/github/v/release/GyverLibs/GyverDB.svg?color=brightgreen)](https://github.com/GyverLibs/GyverDB/releases/latest/download/GyverDB.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GyverDB.svg)](https://registry.platformio.org/libraries/gyverlibs/GyverDB)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GyverDB?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GyverDB
Простая база данных для Arduino:
- Хранение данных в парах ключ-значение
- Поддерживает все целочисленные типы, float, строки и бинарные данные
- Быстрая автоматическая конвертация данных между разными типами
- Быстрый доступ благодаря хэш ключам и бинарному поиску - в 10 раз быстрее библиотеки [Pairs](https://github.com/GyverLibs/Pairs)
- Компактная реализация - 8 байт на одну ячейку
- Встроенный механизм автоматической записи на флешку ESP8266/ESP32

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

### Зависимости
- [StreamIO](https://github.com/GyverLibs/StreamIO)
- [GTL](https://github.com/GyverLibs/GTL) v1.0.6+
- [StringUtils](https://github.com/GyverLibs/StringUtils) v1.4.15+
- [FOR_MACRO](https://github.com/GyverLibs/FOR_MACRO) v1.0.0+

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
Настройки компиляции перед подключением библиотеки
```cpp
#define DB_NO_UPDATES  // убрать стек обновлений
#define DB_NO_FLOAT    // убрать поддержку float
#define DB_NO_INT64    // убрать поддержку int64
#define DB_NO_CONVERT  // не конвертировать данные (принудительно менять тип записи, keepTypes не работает)
```

### GyverDB
```cpp
// конструктор
// можно зарезервировать ячейки
GyverDB(uint16_t reserve = 0);


// не изменять тип записи (конвертировать данные если тип отличается) (умолч. true)
void keepTypes(bool keep);

// использовать стек обновлений (умолч. false)
void useUpdates(bool use);

// было изменение бд
bool changed();

// сбросить флаг изменения бд
void clearChanged();

// вывести всё содержимое БД
void dump(Print& p);

// полный вес БД
size_t size();

// экспортный размер БД (для writeTo)
size_t writeSize();

// экспортировать БД в Stream (напр. файл)
bool writeTo(Stream& stream);

// экспортировать БД в буфер размера writeSize()
bool writeTo(uint8_t* buffer);

// импортировать БД из Stream (напр. файл)
bool readFrom(Stream& stream, size_t len);

// импортировать БД из буфера
bool readFrom(const uint8_t* buffer, size_t len);

// создать запись. Если существует - перезаписать пустой с новым типом
bool create(size_t hash, gdb::Type type, uint16_t reserve = 0);

// полностью освободить память
void reset();

// стереть все записи (не освобождает зарезервированное место)
void clear();

// удалить из БД записи, ключей которых нет в переданном списке
void cleanup(size_t* hashes, size_t len);

// вывести все ключи в массив длиной length()
void getKeys(size_t* hashes);

// получить запись
gdb::Entry get(size_t hash);
gdb::Entry get(const Text& key);

// получить запись по порядку
gdb::Entry getN(int idx);

// удалить запись
void remove(size_t hash);
void remove(const Text& key);

// БД содержит запись с именем
bool has(size_t hash);
bool has(const Text& key);

// записать данные (создать ячейку, если не существует). DATA - любой тип данных
bool set(size_t hash, DATA data);
bool set(const Text& key hash, DATA data);

// инициализировать данные (создать ячейку и записать, если ячейка не существует). DATA - любой тип данных
bool init(size_t hash, DATA data);
bool init(const Text& key hash, DATA data);

// обновить данные (если ячейка существует). DATA - любой тип данных
bool update(size_t hash, DATA data);
bool update(const Text& key hash, DATA data);
```

### GyverDBFile
Данный класс наследует GyverDB, но умеет самостоятельно записываться в файл на флешку ESP при любом изменении и по истечении таймаута
```cpp
GyverDBFile(fs::FS* nfs = nullptr, const char* path = nullptr, uint32_t tout = 10000);

// установить файловую систему и имя файла
void setFS(fs::FS* nfs, const char* path);

// установить таймаут записи, мс (умолч. 10000)
void setTimeout(uint32_t tout = 10000);

// прочитать данные
bool begin();

// обновить данные в файле, если было изменение БД. Вернёт true при успешной записи
bool update();

// тикер, вызывать в loop. Сам обновит данные при изменении и выходе таймаута, вернёт true
bool tick();
```

Для использования нужно запустить FS и вызывать тикер в loop. При любом изменении в БД она сама запишется в файл после выхода таймаута:
```cpp
#include <LittleFS.h>
#include <GyverDBFile.h>
GyverDBFile db(&LittleFS, "db.bin");

void setup() {
    LittleFS.begin();
    db.begin(); // прочитать данные из файла

    // для работы в таком режиме очень пригодится метод init():
    // создаёт запись соответствующего типа и записывает "начальные" данные,
    // если такой записи ещё нет в БД
    db.init("key", 123);    // int
    db.init("uint", 123ul); // uint32
    db.init("str", "init"); // строка
}
void loop() {
    db.tick();
}
```

### Типы записей
```cpp
None
Int
Uint
Int64
Uint64
Float
String
Bin
```

### Entry
- Наследует класс `Text` для более удобного чтения строк

```cpp
// тип записи
gdb::Type type();

// вывести данные в буфер размера size(). Не добавляет 0-терминатор, если это строка
void writeBytes(void* buf);

// вывести в переменную
bool writeTo(T& dest);

Value toText();
String toString();
bool toBool();
int32_t toInt();
int64_t toInt64();
double toFloat();
```

### Использование
#### Ключи
БД хранит ключи в виде хэш-кодов, для доступа к БД нужно использовать непосредственно хэш или обычную строку, библиотека сама посчитает хэш:
```cpp
db["key1"] = 1234;      // строка
db[SH("key2")] = 1234;  // хэш
db["key3"_h] = 1234;    // хэш
```

> Здесь `SH()` - хэш-функция из библиотеки StringUtils, выполняемая на этапе компиляции. Переданная в неё строка не существует в программе - на этапе компиляции она превращается в число. Также можно использовать литерал `_h` - он делает же самое.

Так как хэш - это число, то с базой можно работать и просто обычными числами. Можно считать, что база данных - это массив на 2^29 ячейки:
```cpp
db[0] = 123;
db[2] = 456;
db[100] = "hello";
```

И поэтому вместо хэшей также можно использовать обычный enum:
```cpp
enum keys : size_t {
    key1,
    key2,
    mykey,
    lolkek,
};

db[keys::key1] = 123;
db[keys::key2] = 456;
db[keys::lolkek] = "hello";
```

Это очень удобно, потому что IDE подскажет список имеющихся ключей при вводе `keys::`.

При активной разработке может оказаться удобнее использовать хэш-ключи, так как удаление ключа из обычного enum приведёт к смещению нумерации и под старыми ключами окажутся новые данные. В этом случае enum тоже можно использовать для подсказок IDE, но чуть в другом виде:
```cpp
enum keys : size_t {
    key1 = SH("key1"),
    key2 = SH("key2"),
    mykey = SH("mykey"),
};
```

Теперь enum хранит хэши и не боится удаления или добавления ключей не в конец. Для более короткой записи в библиотеке есть удобный макрос:
```cpp
DB_KEYS(keys,
    key1,
    key2,
    mykey,
);
```
Он развернётся в такой же хэш-enum как в примере выше.

#### Запись и чтение
```cpp
GyverDB db;

// эта ячейка у нас объявлена как int, текст корректно сконвертируется в число
db["key1"] = "123456";

// чтение. Библиотека сама конвертирует в нужный тип
int i = db["key1"];
float f = db[SH("key2")];

// любые данные "печатаются", даже бинарные
Serial.println(db["key3"_h]);

// можно указать конкретный тип при выводе
db["key3"_h].toInt32();

// можно сравнивать с целочисленными
int i = 123;
db["key1"] == i;
db["key1"] >= i;

// сравнение напрямую со строками работает только у записей с типом String
db["key1"] == "str";

// но можно и вот так, для любых типов записей
// toText() конвертирует все типы записей БД во временную строку
db["key1"].toText() == "12345";

// GyverDB может записать данные любого типа, даже составные (массивы, структуры)
uint8_t arr[5] = {1, 2, 3, 4, 5};
db["arr"] = arr;

// вывод обратно. Тип должен иметь такой же размер!
uint8_t arr2[5];
db["arr"].writeTo(arr2);

// посмотрим что записалось
db.dump(Serial);
```

При разработке проекта может оказаться так, что некоторые ключи "устарели" или были переименованы в процессе разработки, и записи по ним уже не нужны. В библиотеке есть возможность провести очистку БД: удалить все лишние записи и оставить только заданный список ключей. Это делается так:
```cpp
  // список ключей, которые надо оставить. В формате size_t в любом виде
  size_t hashes[] = {SH("key1"), "key2"_h, kesy::key3};

  // очищаем
  db.cleanup(hashes, 3);

  // в БД останутся только записи, соответствующие указанным выше ключам
```

### Примечания
- GyverDB хранит целые до 32 бит и float числа в памяти самой ячейки. 64-битные числа, строки и бинарные данные выделяются динамически
- Ради компактности используется 29-битное хэширование. Этого должно хватать более чем, шанс коллизий крайне мал
- Библиотека автоматически выбирает тип записи при записи в ячейку. Приводите тип вручную, если это нужно (например `db["key"] = 12345ull`)
- По умолчанию включен параметр `keepTypes()` - не изменять тип записи при перезаписи. Это означает, что если запись была int, то при записи в неё данных другого типа они будут автоматически конвертироваться в int, даже если это строка. И наоборот
- При создании пустой ячейки можно указать тип и зарезервировать место (только для строк и бинарных данных) `db.create("kek", gdb::Type::String, 100)`
- `Entry` имеет автоматический доступ к строке как оператор `String`, это означает что записи с текстовым типом (String) можно передавать в функции, которые принимают `String`, например `WiFi.begin(db["wifi_ssid"], db["wifi_pass"]);`
- Если нужно передать запись в функцию, принимающую `const char*` - используйте на ней `c_str()`. Это не продублирует строку в памяти, а даст к ней прямой доступ. Например `foo(db["str"].c_str())`

<a id="versions"></a>

## Версии
- v1.0
- v1.0.1 упразднены целые типы 8 и 16 бит, увеличено разрешение хэша

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **GyverDB** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/GyverDB/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>

## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!

При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код
