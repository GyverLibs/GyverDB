This is an automatic translation and may be incorrect in some places. See the source README and examples for authoritative information.

[![latest](https://img.shields.io/github/v/release/GyverLibs/GyverDB.svg?color=brightgreen)](https://github.com/GyverLibs/GyverDB/releases/latest/download/GyverDB.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GyverDB.svg)](https://registry.platformio.org/libraries/gyverlibs/GyverDB)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GyverDB?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GyverDB
A simple database for Arduino:
- Storage of data in key-value pairs
- Supports all integer types, float, strings and binary data
- Automatic data conversion between different types
- Quick access thanks to hash keys and binary search – 10 times faster than a library[Pairs](https://github.com/GyverLibs/Pairs)11 times faster than Preferences (ESP32)
- Compact implementation - 8 bytes per cell
- Built-in automatic recording mechanism on the ESP8266/ESP32 flash drive

### Compatibility
Compatible with all Arduino platforms (Arduino features are used)

### Dependencies
- [StreamIO](https://github.com/GyverLibs/StreamIO)
- [GTL](https://github.com/GyverLibs/GTL) v1.0.6+
- [StringUtils](https://github.com/GyverLibs/StringUtils) v1.4.15+
- [FOR_MACRO](https://github.com/GyverLibs/FOR_MACRO) v1.0.0+

## Contents
- [Documentation.](#docs)
- [Use of use](#usage)
- [Versions](#versions)
- [Installation](#install)
- [Bugs and feedback](#feedback)

<a id="docs"></a>

## Documentation.
Compilation settings before connecting the library
```cpp
#define DB_NO_UPDATES  // remove the update stack
#define DB_NO_FLOAT    // remove float support
#define DB_NO_INT64    // Remove Int64 Support
#define DB_NO_CONVERT  // not convert data (forced to change cell type, keepTypes does not work)
```

### GyverDB
```cpp
// designer
// We can reserve the cells.
GyverDB(uint16_t reserve = 0);

// Do not change the cell type (convert data if the type is different)
void keepTypes(bool keep);

// There was a change in BD
bool changed();

// reset the change flag bd
void clearChanged();

// remove all contents of the database
void dump(Print& p);

// full-weight
size_t size();

// Export size of the database (for writeTo)
size_t writeSize();

// export the database to Stream (e.g. file)
bool writeTo(Stream& stream);

// Export the database to the writeSize buffer()
bool writeTo(uint8_t* buffer);

// import databases from Stream (e.g. file)
bool readFrom(Stream& stream, size_t len);

// buffer
bool readFrom(const uint8_t* buffer, size_t len);

// create a cell. If there is, re-write the empty with a new type.
bool create(size_t hash, gdb::Type type, uint16_t reserve = 0);

// free up
void reset();

// erase all cells (does not vacate the reserved space)
void clear();

// remove from the database cells whose keys are not in the transferred list
void cleanup(size_t* hashes, size_t len);

// Put all keys in the array length()
void getKeys(size_t* hashes);

// cellize
gdb::Entry get(size_t hash);
gdb::Entry get(const Text& key);

// sequence
gdb::Entry getN(int idx);

// remove
void remove(size_t hash);
void remove(const Text& key);

// The database contains a cell with a name
bool has(size_t hash);
bool has(const Text& key);

// Write data (create a cell if it does not exist). DATA - any type of data
bool set(size_t hash, DATA data);
bool set(const Text& key, DATA data);

// initialize data (create a cell and write if the cell does not exist). DATA - any type of data
bool init(size_t hash, DATA data);
bool init(const Text& key, DATA data);

// Update the data (if the cell exists). DATA - any type of data
bool update(size_t hash, DATA data);
bool update(const Text& key, DATA data);

// connect the processor to create and change the write value of the form void f(size t hash)
void onChange(ChangeCallback cb);

// Use the update stack (silent. false)
void useUpdates(bool use);

// There are unread changes
bool updatesAvailable();

// skip raw updates
void skipUpdates();

// Get a hash update from the stack
size_t updateNext();
```

### GyverDBFile
This class inherits GyverDB, but can independently write to the file on the ESP flash drive at any change and after the timeout.

```cpp
GyverDBFile(fs::FS* nfs = nullptr, const char* path = nullptr, uint32_t tout = 10000);

// install the file system and file name
void setFS(fs::FS* nfs, const char* path);

// Set timeout records, MS (silent 10000)
void setTimeout(uint32_t tout = 10000);

// read
bool begin();

// Update the data in the file if there was a change in the database. Return true upon successful recording
bool update();

// ticker, call the loop. You will update the data when you change and exit the timeout, return the true
bool tick();
```

To use, you need to run the FS and call the ticker in the loop:

```cpp
#include <LittleFS.h>
#include <GyverDBFile.h>
GyverDBFile db(&LittleFS, "data.db");

void setup() {
    LittleFS.begin();
    db.begin(); // file-read

    // To work in this mode, the init() method is useful:
    // creates a cell of the appropriate type and records the "initial" data.
    // If such a cell is not already in the DB
    db.init("key", 123);    // int
    db.init("fl", 3.14);    // float
    db.init("str", "init"); // line
}
void loop() {
    db.tick();
}
```

- With any change in the database, it will record itself in the file after the timeout.
- The database is in RAM for quick access, it is read from the file only when called`begin`
- The file extension does not matter - it is more a hint to the user that this file stores the database. The file contains the database in *binary form* - it can not be edited through a notebook!

### Types of cells gdb:::Type
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
```cpp
// cell
gdb::Type type();

// output data to the size() buffer. Does not add a 0-terminator if it is a string
void writeBytes(void* buf);

// variable
bool writeTo(T& dest);

Value toText();
String toString();
bool toBool();
int32_t toInt();
int64_t toInt64();
double toFloat();
```

<a id="usage"></a>

## Use of use
GyverDB is a dynamic database that stores data in key-value format. By key, you can write data to the cell and read them from it:

- Key - 29 bit number, in fact database is an array of 2^29 cells
- Value - data of any type: numbers, strings, any binary data

```cpp
db[0] = 123;
db[2] = 3.14;
db[100] = "hello";
```

As long as nothing is written into the cell, it does not exist and does not occupy memory. The key should be treated as a **unique cell identifier**, not as a sequence number in an array - an index.

### Keys.
To increase the readability of the code instead of cell numbers, it is more convenient to use constants, for example.`enum`. This is very convenient, because the IDE will tell you the list of available keys when entering.`keys::`and the compiler will substitute the values:

```cpp
enum keys : size_t {
    key1,
    key2,
    mykey,
    lolkek,
};

db[keys::key1] = 123;
db[keys::key2] = 3.14;
db[keys::mykey] = "hello";
```

With active development and storage of the database in non-volatile memory (in a file, reading when loading the MK), this approach is inconvenient, since deleting or adding a key in the middle of the list will lead to a shift in numbering and new data will be under the old keys. To preserve the readability and uniqueness of each key, you can use hash lines - a line with a special function is converted to a number that corresponds only to this line. This feature is built into GyverDB - you can access the cells by string key:

```cpp
db["key1"] = 123;
```

To speed up and facilitate the code, you can use an external hash function, which is executed at the compilation stage and immediately turns into a number. Together with GyverDB there are several options, they are equivalent:

```cpp
db[SH("key1")] = 123;
db["key2"_h] = 3.14;
db[H(mykey)] = "hello";
```

In this case, enum can also be used for IDE tips, but in a slightly different form:

```cpp
// enume
enum keys : size_t {
    key1 = "key1"_h,
    key2 = SH("key2"),
    mykey = H(mykey),
};

db[keys::key1] = 123;
db[keys::key2] = 3.14;
db[keys::mykey] = "hello";
```

Now enum stores hashes and is not afraid to delete or add keys - they are unique. For a shorter entry in the library there is a convenient macro:

```cpp
DB_KEYS(keys,
    key1,
    key2,
    mykey   // last comma not placed
);
```

It will unfold in the same hash enum as in the example above. It is recommended to use this option as the most convenient and optimal.

> There's more.`DB_KEYS_CLASS`- He creates.`enum class`. But such constants will need to be manually cast to`size_t`

#### Write and read
```cpp
GyverDB db;

// WRITE
// directly. When creating the cell will receive the type Int
db["key1"] = 123;

// This cell is int, the text is converted into a number.
db["key1"] = "123456";

// It’s a little more efficient to write through set.
db.set("key1", 123321);
db.set("key2", "3.14");
```
```cpp
// Reading
// The cell itself converts to the type to the left of the sign.
int i = db["key1"];
float f = db.get("key2");   // It's a little more effective to read through

// any data is printed in Print, even binary
Serial.println(db["key3"]);

// can be converted to a specific type
i = db["key1"].toInt();
i = db["key2"].toBool();
f = db["key3"].toFloat();

// numerical
db["key1"] == 123;
db["key1"] >= 123;

// Compound operators and increment/decrement
db["key1"]++;
db["key1"] += 10;
db["key1"] &= 0x12;

// String records can be compared to strings
db["key2"] == "str";

// But you can do this for any type of cell.
// toText() converts all types of database cells to a timeline
db["key1"].toText() == "12345";
```
```cpp
// BINARY
// GyverDB can record data of any type, even composite (arrays, structures)
uint8_t arr[5] = {1, 2, 3, 4, 5};
db["arr"] = arr;

// pull back. The type should be the same size!
uint8_t arr2[5];
db["arr"].writeTo(arr2);

// Printing
db.dump(Serial);
```
```cpp
// Structures
struct Foo {
    int a;
    float b;
};

Foo foo{123, 3.14};
db["struct"] = foo;

// copy-read
Foo foo2;
db["struct"].writeTo(foo2);
Serial.println(foo2.b);

// direct
Serial.println(static_cast<Foo*>(db["struct"].buffer())->a);  // 123

Foo& ref = *static_cast<Foo*>(db["struct"].buffer());  // 3.14
Serial.println(ref.b);

// stratum
Foo arr[] = {{123, 3.14}, {456, 2.72}};
db["arr"] = arr;

Foo* p = (Foo*)db["arr"].buffer();
Serial.println(p[0].a);  // 123
Serial.println(p[1].b);  // 2.72
```

When developing a project, it may turn out that some keys are “outdated” or have been renamed during the development process, and cells on them are no longer needed. In the library there is an opportunity to clean the database: remove all extra cells and leave only a specified list of keys. It's done like this:
```cpp
// A list of keys to leave behind. In size t format in any form
size_t hashes[] = {SH("key1"), "key2"_h, kesy::key3};

// clean
db.cleanup(hashes, 3);

// Only the cells corresponding to the above keys will remain in the database.
```

There are 4 options for recording in the cell:

- `create(ключ, тип)`- create an empty cell of the specified type. If a cell with such a key exists, clean and change the type.
- `init(ключ, значение)`create a cell with a specified value if there is no cell with such a key or it has a different type of data. Convenient for setting initial values in GyverDBFile
- `update(ключ, значение)`Update the data if a cell with such a key exists
- `set(ключ, значение)`Record data by creating a cell if it does not exist. Analogue`db[ключ] = значение`

A shorter macro can be used to initialize:

```cpp
DB_INIT(
    db,
    (keys::key1, 123),
    (keys::key2, 3.14),
    ("key3", 123321ull),
    ("key4", "abc")
);
```

### Notes
- GyverDB stores up to 32 bits and float numbers in the memory of the cell itself. 64-bit numbers, strings and binary data are allocated dynamically
- For the sake of compactness, 29-bit hashing is used. This should be more than enough, the chance of collisions is extremely small.
- The library automatically selects the type when writing to the cell. Handle the type if needed (e.g.`db["key"] = 12345ull`)
- By default, the parameter is enabled`keepTypes()`- save the cell type when overwriting. This means that if the cell was int, then when another type of data is written to it, it will automatically convert to int, even if it is a string. And vice versa.
- When creating an empty cell, you can specify the type and reserve space (for strings and binary data only)`db.create("kek", gdb::Type::String, 100)`
- `Entry`has automatic access to the line as an operator`String`This means that cells with a text type (String) can be transferred to functions that receive`String`for example`WiFi.begin(db["wifi_ssid"], db["wifi_pass"]);`
- If you need to transfer the cell to a function that receives`const char*`- Use it on it.`c_str()`. This will not duplicate a line in memory, but will give it direct access. For example,`foo(db["str"].c_str())`

<a id="versions"></a>

## Versions
- v1.0
- v1.0.1 abolished whole types of 8 and 16 bits, increased hash resolution
- v1.2.1

<a id="install"></a>
## Installation
- The library can be found under the name **GyverDB** and installed through the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download the library](https://github.com/GyverLibs/GyverDB/archive/refs/heads/main.zip).zip archive for manual installation:
    - Unpack and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unpack and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/ *
    - (Arduino IDE) Automatic installation from .zip: *Sketch/Connect library/Add .ZIP library...* and specify downloaded archive
- Read more detailed instructions for installing libraries[here](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: new versions fix errors and bugs, as well as optimize and add new features.
- Through the library manager IDE: find the library as when installing and click "Update"
- Manually: **Delete the folder with the old version** and then put the new one in its place. “Replacement” can not be done: sometimes new versions delete files that will remain when replaced and can lead to errors!

<a id="feedback"></a>

## Bugs and feedback
If you find bugs, create **Issue**, or better write to the mail immediately.[alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
The library is open for revision and your **Pull Requests*!

When reporting bugs or incorrect work of the library, it is necessary to specify:
- Library version
- What is used by the IC
- SDK version (for ESP)
- Arduino IDE version
- Are embedded examples that use features and designs that cause bugs in your code working correctly?
- What code was downloaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed. Not a canvas of a thousand lines, but a minimum code.
