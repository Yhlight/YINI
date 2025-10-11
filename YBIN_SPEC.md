# YINI Binary Asset (.ybin) Specification

## 1. Overview

The `.ybin` format is designed for high-performance, zero-parsing loading of YINI configurations in a game engine or application. It is a "cooked" format, meaning it stores the fully resolved configuration data in a contiguous block of memory, suitable for direct memory mapping.

## 2. Goals

- **Performance**: Near-instantaneous loading with minimal CPU overhead.
- **Simplicity**: Easy to read and process with a simple loader.
- **Compactness**: Efficiently stores data to reduce file size.
- **Versioning**: Supports future changes to the format.

## 3. File Structure

The `.ybin` file is composed of several main parts, laid out in the following order:

1.  **File Header**: Contains metadata about the file, such as a magic number and version.
2.  **Section Table**: An index of all sections in the file for quick lookups.
3.  **Key/Value Table**: Contains all the key-value pairs, grouped by section.
4.  **String Pool**: A dedicated area for storing all unique strings (section names, keys, and string values) to avoid duplication.
5.  **Data Block**: A tightly packed region for non-string values (integers, floats, booleans, etc.).

---

### 3.1. File Header (16 bytes)

| Offset | Size | Type    | Description                               |
|--------|------|---------|-------------------------------------------|
| 0      | 4    | `char[4]` | Magic Number: `YINI`                      |
| 4      | 4    | `uint32`  | Format Version: e.g., `1`                 |
| 8      | 4    | `uint32`  | Offset to the Section Table               |
| 12     | 4    | `uint32`  | Number of Sections                        |

---

### 3.2. String Pool

The String Pool is a simple, contiguous block of memory containing all unique strings, null-terminated. All references to strings within the file (e.g., section names, keys) are stored as offsets into this pool.

---

### 3.3. Section Table

The Section Table is an array of `SectionEntry` structs. Its location is specified in the file header.

#### `SectionEntry` Struct (12 bytes)

| Offset | Size | Type    | Description                               |
|--------|------|---------|-------------------------------------------|
| 0      | 4    | `uint32`  | Offset into the String Pool for the section name |
| 4      | 4    | `uint32`  | Offset to the first Key/Value entry for this section |
| 8      | 4    | `uint32`  | Number of Key/Value pairs in this section |

---

### 3.4. Key/Value Table

This table contains the metadata for every key-value pair. It's an array of `KeyValueEntry` structs, grouped by section.

#### `ValueType` Enum (1 byte)

A simple enum to identify the type of the value.
- `0x01`: INT
- `0x02`: FLOAT
- `0x03`: BOOL
- `0x04`: STRING (value is an offset into the String Pool)
- `0x05`: ARRAY_INT
- ... (extensible for other types)

#### `KeyValueEntry` Struct (9 bytes)

| Offset | Size | Type      | Description                               |
|--------|------|-----------|-------------------------------------------|
| 0      | 4    | `uint32`    | Offset into the String Pool for the key name |
| 4      | 1    | `ValueType` | The type of the value                     |
| 5      | 4    | `uint32`    | Offset into the Data Block where the value is stored |

---

## 4. Loading Mechanism

A loader can read a `.ybin` file by:
1.  Reading the header to verify the magic number and version.
2.  Memory-mapping the entire file into memory.
3.  Using the Section Table to quickly find a desired section by name.
4.  Iterating through the `KeyValueEntry` structs for that section to find a desired key.
5.  Based on the `ValueType`, retrieving the value from the appropriate location (either the Data Block or the String Pool).

This design ensures that once the file is mapped, no further allocations or complex parsing are needed to access any configuration value.
