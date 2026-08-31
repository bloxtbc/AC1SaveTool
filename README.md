# AC1 Save Tool

A command-line tool for reading, writing, exporting, and importing **Assassin's Creed 1** save files.

The project is designed around the game's `BinSerializer` format and currently supports converting the binary save format to an editable JSON representation and converting it back to the original binary format.

> **Status:** Work in progress. The binary format is actively being reverse engineered, and some serializer field types are not yet fully understood.

---

## Features

* Read AC1 binary save files
* Write AC1 binary save files
* Export saves to JSON
* Import JSON back into binary saves
* Big-endian binary serialization
* Property metadata and dynamically resolved property types
* Hash database for resolving property IDs to readable names
* JSON representation of vectors, matrices, strings, arrays, and primitive values
* Byte-for-byte JSON round-trip testing
* CTest integration

---

## Requirements

### Compiler

A compiler supporting **C++20** is required.

Tested with:

* GCC / MinGW
* CMake
* Ninja

Other C++20 compilers should work as well.

### Dependencies

The project uses:

* [nlohmann/json](https://github.com/nlohmann/json)

The JSON library is included as a Git submodule under:

```text
external/json
```

---

## Building

Clone the repository including submodules:

```bash
git clone --recurse-submodules <repository-url>
cd AC1SaveTool
```

If the repository was already cloned without submodules:

```bash
git submodule update --init --recursive
```

### Configure

Using CMake:

```bash
cmake -S . -B build -G Ninja
```

### Build

```bash
cmake --build build
```

The executable will be placed in the CMake build output directory, for example:

```text
build/Debug/ac1save.exe
```

or:

```text
build/ac1save.exe
```

depending on the selected generator/configuration.

---

## CLI Usage

The general syntax is:

```text
ac1save <command> <input> <output>
```

### Export a save to JSON

```bash
ac1save export testSave.sav testSave.json
```

This reads the binary save file and creates an editable JSON representation.

Example:

```text
Exporting save
  Input:  "testSave.sav"
  Output: "testSave.json"
  Size:   177399 bytes
  Magic:  0x1234FEDC
  Version: 15
  Objects: 950
  JSON:   2439941 bytes

Export complete.
```

---

### Import JSON into a save

```bash
ac1save import testSave.json modified.sav
```

The JSON is parsed back into the internal save representation and serialized using the AC1 binary format.

For example:

```text
Importing save
  Input:  "testSave.json"
  Output: "modified.sav"
  JSON:   2439941 bytes

Import complete.
```

You can then place the resulting save file back into the appropriate AC1 save directory.

---

## JSON Format

The JSON format is intended to be easier to inspect and edit than the raw binary format.

A simplified save looks like:

```json
{
  "fileSize": 177399,
  "magic": "0x1234FEDC",
  "version": 15,
  "objectCount": 950,
  "objects": [
    {
      "index": 0,
      "classID": 2101,
      "propertyCount": 10,
      "unknown": 0,
      "unknown2": 0,

      "properties": [
        {
          "propType": 1,
          "id": 494490051,
          "name": "CitiesInKingdom",
          "index": 0,
          "payload": {
            "value": 123
          }
        }
      ],

      "metadata": [
        {
          "unknown": 0,
          "type": "UInt32"
        }
      ],

      "values": [
        {
          "type": "UInt32",
          "value": 123
        }
      ]
    }
  ]
}
```

The exact structure may change as additional parts of the AC1 serialization system are reverse engineered.

---

## Hash Database

Many AC1 properties are represented internally using 32-bit hash values.

For example:

```text
494490051
```

can be resolved to:

```text
CitiesInKingdom
```

The project supports a hash database stored as:

```text
hashes/hashes.json
```

The database allows exported JSON to contain both the numeric ID and its known name:

```json
{
  "id": 494490051,
  "name": "CitiesInKingdom"
}
```

Unknown hashes remain numeric.

### Adding hashes

The repository contains tooling for extracting hashes from XML exports produced by other AC1 tools.

Multiple XML files can be processed and merged into the hash database.

This allows the database to grow as more game data is discovered.

---

## Testing

The project uses **CTest**.

Build the project first:

```bash
cmake --build build
```

Then run:

```bash
ctest --test-dir build --output-on-failure
```

### Tests

The project currently includes tests for:

#### Binary round trip

Tests:

```text
Save file
    ↓
SaveReader
    ↓
SaveGame
    ↓
SaveWriter
    ↓
Save file
```

The resulting binary is compared against the original.

#### JSON round trip

The more important integration test is:

```text
testSave.sav
    ↓
SaveReader
    ↓
SaveGame
    ↓
SaveJsonExporter
    ↓
JSON
    ↓
SaveJsonImporter
    ↓
SaveGame
    ↓
SaveWriter
    ↓
testSave_roundtrip.sav
```

The final file is compared **byte-for-byte** against the original.

A successful test looks like:

```text
100% tests passed, 0 tests failed
```

This ensures that exporting and importing a save does not unintentionally modify the binary representation.

---

## Project Structure

The project is organized into several layers:

```text
AC1SaveTool/
│
├── include/
│   └── ac1/
│       ├── model/
│       ├── io/
│       └── ...
│
├── src/
│   ├── model/
│   ├── io/
│   ├── json/
│   └── ...
│
├── tests/
│   ├── assets/
│   │   └── testSave.sav
│   ├── test_roundtrip.cpp
│   └── test_json_roundtrip.cpp
│
├── hashes/
│   ├── hashes.json
│   └── ...
│
├── external/
│   └── json/
│
├── CMakeLists.txt
└── README.md
```

The exact directory layout may evolve as the project is expanded.

---

## Architecture

The tool is divided into several major components.

### BinaryReader

Responsible for reading primitive values from the AC1 binary format.

AC1 values are read as **big-endian**:

```cpp
reader.u32be();
reader.u64be();
reader.f32be();
```

### BinaryWriter

Performs the corresponding big-endian serialization:

```cpp
writer.u32be(value);
writer.u64be(value);
writer.f32be(value);
```

### SaveReader

Converts the binary AC1 save format into the internal C++ representation:

```text
Binary
  ↓
BinaryReader
  ↓
SaveReader
  ↓
SaveGame
```

### SaveWriter

Converts the internal representation back into the AC1 binary format:

```text
SaveGame
  ↓
SaveWriter
  ↓
BinaryWriter
  ↓
Binary
```

### JSON Exporter

Converts `SaveGame` into a human-readable JSON representation.

### JSON Importer

Performs the reverse operation:

```text
JSON → SaveGame
```

The JSON importer and exporter should share the same serializer type definitions and conversion logic wherever possible.

---

## Serializer Field Types

AC1 properties have a serializer field type encoded inside their property metadata.

Currently identified types include:

```text
0x00  Bool
0x01  UInt8
0x02  Int8Alt1
0x03  Int8Alt2
0x04  UInt16
0x05  Int16
0x06  Float32
0x07  UInt32
0x08  Float64
0x09  UInt64
0x0A  UInt32Alt
0x0B  Vector2
0x0C  Vector3
0x0D  Vector4
0x0E  UInt32X4X4
0x0F  Matrix3x3
0x10  Matrix4x4
0x11  HashOrId
0x12  Unknown12
0x13  Unknown13
0x14  Unknown14
0x15  String
0x16  Pointer
0x17  Struct
0x18  Array
0x19  UInt32Alt2
0x1A  Unknown1A
0x1B  Unknown1B
0x1C  Unknown1C
0x1D  ArrayAlt
```

Some types are still under investigation.

Unsupported types should not be silently interpreted because doing so could corrupt the resulting save.

---

## Editing Saves

The intended workflow is:

```text
Original save
     │
     ▼
   export
     │
     ▼
   JSON
     │
     │  edit values
     ▼
 modified JSON
     │
     ▼
   import
     │
     ▼
Modified save
```

For example:

```bash
ac1save export mysave.sav mysave.json
```

Edit:

```text
mysave.json
```

Then:

```bash
ac1save import mysave.json mysave_modified.sav
```

Always keep a backup of the original save.

---

## Development

Configure a development build:

```bash
cmake -S . -B build -G Ninja
```

Build:

```bash
cmake --build build
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Run a specific test:

```bash
ctest --test-dir build -R JsonRoundTrip --output-on-failure
```

---

## Reverse Engineering Notes

This project is based on reverse engineering the Assassin's Creed 1 serialization system, including the game's `BinSerializer`, property metadata, serializer field types, and save-game object layout.

The format is still being documented.

Unknown fields are intentionally retained rather than discarded where possible so that the tool can preserve files without requiring every field to be understood.

The primary goal is **lossless serialization first**, followed by making the format convenient to edit.

---

## Disclaimer

This project is an unofficial reverse-engineering/modding tool and is not affiliated with or endorsed by Ubisoft.

Use it at your own risk and keep backups of your save files.
