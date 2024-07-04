This is an automatic translation, may be incorrect in some places. See sources and examples!

# Gyverdb
Simple database for Arduino:
- storage of data in the key pairs- value
- supports all integer types, Float, lines and binary data
- Fast automatic data conversion between different types
- Fast access thanks to the hash keys and binary search - 10 times faster than the library [PAIRS] (https://github.com/gyverlibs/pairs)
- compact implementation - 8 bytes per cell
- Built -in automatic recording mechanism for ESP8266/ESP32 flash drive

## compatibility
Compatible with all arduino platforms (used arduino functions)

### Dependencies
- [gtl] (https://github.com/gyverlibs/gtl) v1.0.6+
- [Stringutils] (https://github.com/gyverlibs/stringutils) v1.4.15+

## Content
- [use] (#usage)
- [versions] (#varsions)
- [installation] (# Install)
- [bugs and feedback] (#fedback)

<a id="usage"> </a>

## Usage
Compilation settings before connecting the library
`` `CPP
#define db_no_updates // Remove glass updates
#define db_no_float // Remove support for Float
#define db_no_int64 // Remove support for int64
#define db_no_convert // Do not convert data (forcibly change the type of record, KeeptyPes does not work)
`` `

## gyverdb
`` `CPP
// Designer
// you can reserve cells
Gyverdb (uint16_t reserve = 0);


// Do not change the type of recording (convert data if the type is different) (silence. True)
VOID Keeptypes (Bool Keep);

// Use the glass updates (silence. FALSE)
VOID USEUPDATES (BOOL USE);

// there was a change in data.After the response, it will drop in FALSE
Bool Changed ();

// Determine all the contents of the database
VOID DUMP (Print & P);

// Full weight of the database
size_t size ();

// Export size of the database (for writeto)
Size_t Writesize ();

// export the database in Stream (e.g. file)
Bool Writeto (Stream & Stream);

// export the database in the size of the size of writesize ()
Bool Writeto (Uint8_t* Buffer);

// import a database from Stream (e.g. file)
Bool Readfrom (Stream & Stream, Size_t Len);

// import a databar from the buffer
Bool Readfrom (Consta Uint8_t* Buffer, Size_t Len);

// Create a record.If exists - rewrite empty with a new type
Bool Create (Size_t Hash, GDB :: Type Type, Uint16_t Reserv = 0);

// completely free memory
VOID Reset ();

// erase all notes (does not free up the reserved place)
Void Clear ();

// get an entry
GDB :: Entry Get (Size_t Hash);
GDB :: Entry Get (Constra Text & Key);

// Delete the recording
VOID REMOVE (SIZE_T HASH);
VOID Remove (Const Text & Key);

// database contains an entry with the name
Bool has (size_t hash);
Bool Has (Consta Text & Key);

// Write data.Data - Any Type of Data
Bool Set (Size_t Hash, Data);
Bool Set (Consta Text & Key Hash, Data);

// initialize the data (create a cell and write down if it is not).Data - Any Type of Data
Bool Set (Size_t Hash, Data);
Bool Set (Consta Text & Key Hash, Data);
`` `

## gyverdbfile
This class inherits Gyverdb, but knows how to independently sign up for an ESP flash drive with any change and after the time of the time
`` `CPP
Gyverdbfile (fs :: fs* nfs = nullptr, const char* PATH = NULLPTR, UINT32_T TOUT = 10000);

// Installsystem and file name
VOID setfs (fs :: fs* nfs, const char* Path);

// Install the Takeout of the Records, MS (silence 1000)
VOID settimeout (uint32_t tout = 10000);

// Read the data
Bool Begin ();

// update data in the file now
Bool update ();

// ticker, call in LOOP.He will update the data himself when the timuta is changed and output, it will return True
Bool Tick ();
`` `

For use, you need to start FS and call a ticker in LOOP.With any change in the database, it itself will be written to the file after the time of the time:
`` `CPP
#include <Littlefs.h>
#include <gyverdbfile.h>
Gyverdbfile DB (& Littlefs, "DB.BIN");

VOID setup () {
Littlefs.Begin ();
db.begin ();// read data from the file

// For work in this mode, the Init () method is very useful:
// creates a record of the corresponding type and writes out "initial" data,
// If such a record is not yet in the database
db.init ("key", 123);// int
db.init ("uint", (uint8_t) 123);// uint8
db.init ("str", "");// line
}
VOID loop () {
db.tick ();
}
`` `

### Types of records
`` `CPP
None
Int8
Uint8
Int16
Uint16
Int32
Uint32
Int64
Uint64
Float
String
Bin
`` `

### Entry
- Inherits the class `text` for more convenient reading of lines

`` `CPP
// type of record
GDB :: Type Type ();

// Bring the data to the size of size ().Does not add a 0-terminator if this is a line
VOID Writebytes (VOID* BUF);

// bring to the variable
Bool Writeto (T & Dest);

Value Totext ();
String Tostring ();
Bool Tobool ();
int toint ();
int8_t toint8 ();
int16_t toint16 ();
int32_t toint32 ();
int64_t toint64 ();
Double Tofloat ();
`` `

### Usage
The database stores the keys in the form of a hash code from the Stringutils library, to access the database you need to use a hash or a regular line, the library itself will consider the hash:
`` `CPP
db ["key1"];// line
DB [SH ("Key2")];// Hash
db ["key3" _h];// Hash
`` `

Here is `SH ()` - a hash -function performed at the compilation stage.The line transferred to it does not exist in the program - at the compilation stage, it turns into the number.You can also use the literal `_H` - it does the same.

Record and reading
`` `CPP
Gyverdb DB;

// Record
db ["key1"] = 1234;
DB [SH ("Key2")] = 1234;
db ["key3" _h] = 1234;

// This cell is declared as int, the text is correctly adjusted to the number
db ["key1"] = "123456";

// Reading.The library itself converts into the right type
int i = db ["key1"];
Float F = DB [SH ("Key2")];

// Any data "printed", even binary
Serial.println (db ["key3" _h]);

// you can specify a specific type when output
db ["key3" _h] .toint32 ();

// can be compared with integer
int i = 123;
123 == DB ["key1"];

// Comparison directly with lines works only in records with the type of string
db ["key1"] == "str";

// but you can and like this, for any type of records
// Totext () converts all types of database entries into a temporary line
db ["key1"]. Totext () == "12345";

// gyverdb can record data from any type, even composite (arrays, structures)
uint8_t arr [5] = {1, 2, 3, 4, 5};
db ["arr"] = arr;

// Conclusion back.The type must have the same size!
uint8_t arr2 [5];
db ["arr"]. Writeto (arr2);
`` `

In a large project, remembering all the names of the keys is not very convenient, so for a more comfortable development you can make the base of the Hash keys using `enum`:
`` `CPP
enum keys: size_t {
Key1 = SH ("Key1"),
key2 = "key1" _h,
Mykey = "Mykey" _H,
};
`` `

And use them in the code as `keys :: key1`, for example` db [keys :: mykey] `.IDE will tell you a list of all the keys when entering `keys ::`.To reduce the recording of the key base, you can use the built -in macros:
`` `CPP
Db_keys (keys,
Db_key (my_key),
Db_key (key1),
Db_key (wifi_ssid),
);
`` `
Macro unfolds in the same Enum as shown above.

### Notes
- Gyverdb stores whole up to 32 bits and Float numbers in the memory of the cell itself.64-bit numbers, lines and binary data are distinguished dynamically
- For the sake of compactness, a 28-bit hash is used.This should be enough more than, the chance of conflicts is extremely small
- The library automatically selects the type of record when recording in the cell.Bring the typemanually, if necessary (for example, `DB [" key "] = (uint32_t) 12345`)
- By default, the `KeeptyPes ()` parameter is included not to change the type of recording during rewriting.This means that if the recording was Int16, then when recording data from another type in it, they will automatically convert to Int16, even if it is a line.And vice versa
- When creating an empty cell, you can specify the type and reserve a place (only for lines and binary data) `DB.create (" KEK ", GDB :: type :: string, 100)` `100)`
- `Entry` has automatic access to the line as an` string` operator, this means that records with a text type can be transmitted to functions that take `string`, for example` wifi.begin (db ["wifi_ssid"], db["wifi_pass"]); `
- If you need to transfer the record to the function that accepts `Const Char*` - use `c_str ()` on it.This will not duplicate the line in memory, but will give direct access to it.For example `foo (db [" str "]. C_str ())`

<a id="versions"> </a>

## versions
- V1.0

<a id="install"> </a>
## Installation
- The library can be found by the name ** gyverdb ** and installed through the library manager in:
- Arduino ide
- Arduino ide v2
- Platformio
- [download the library] (https://github.com/gyverlibs/gyverdb/archive/refs/heads/main.zip) .Zip archive for manual installation:
- unpack and put in * C: \ Program Files (X86) \ Arduino \ Libraries * (Windows X64)
- unpack and put in * C: \ Program Files \ Arduino \ Libraries * (Windows X32)
- unpack and put in *documents/arduino/libraries/ *
- (Arduino id) Automatic installation from. Zip: * sketch/connect the library/add .Zip library ... * and specify downloaded archive
- Read more detailed instructions for installing libraries [here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%BD%D0%BE%BE%BE%BED0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!

<a id="feedback"> </a>

## bugs and feedback
Create ** Issue ** when you find the bugs, and better immediately write to the mail [alex@alexgyver.ru] (mailto: alex@alexgyver.ru)
The library is open for refinement and your ** pull Request ** 'ow!

When reporting about bugs or incorrect work of the library, it is necessary to indicate:
- The version of the library
- What is MK used
- SDK version (for ESP)
- version of Arduino ide
- whether the built -in examples work correctly, in which the functions and designs are used, leading to a bug in your code
- what code has been loaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed.Not a canvas of a thousand lines, but a minimum code