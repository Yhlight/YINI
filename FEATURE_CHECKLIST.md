# YINI Language Feature Checklist

This checklist is derived from the `YINI.md` specification and will be used to audit the project's implementation.

## 1. Core Syntax & Concepts

- [ ] **Comments**:
  - [ ] Single-line comments using `//`.
  - [ ] Multi-line (block) comments using `/* ... */`.
- [ ] **File Extension**:
  - [ ] `.yini` or `.YINI` are recognized.
- [ ] **Case Sensitivity**:
  - [ ] Keywords like `color` and `Color` are treated the same.
  - [ ] Section and key identifiers are case-sensitive.

## 2. Sections and Inheritance

- [ ] **Section Declaration**:
  - [ ] Basic syntax `[SectionName]`.
  - [ ] Top-level key-value pairs outside a section are a syntax error.
- [ ] **Inheritance**:
  - [ ] Syntax `[Child] : Parent1, Parent2`.
  - [ ] Values from parent sections are correctly inherited.
  - [ ] Values from later parents in the list override earlier ones.
  - [ ] Values in the child section override inherited values.

## 3. Value Types and Literals

- [ ] **Primitive Types**:
  - [ ] Integers (e.g., `123`).
  - [ ] Floating-point numbers (e.g., `3.14`).
  - [ ] Booleans (`true`/`false`).
  - [ ] Strings (e.g., `"value"`).
- [ ] **Collection Types**:
  - [ ] **Array**:
    - [ ] `[1, 2, 3]` syntax.
    - [ ] `array(1, 2, 3)` syntax.
    - [ ] Nested arrays `[[1], [2]]`.
  - [ ] **Set**:
    - [ ] `(1, 2, 3)` syntax.
    - [ ] Single-element set with trailing comma `(value, )`.
  - [ ] **List**:
    - [ ] `list(1, 2, 3)` syntax.
  - [ ] **Struct**:
    - [ ] `{key: value}` syntax (single pair, no trailing comma).
  - [ ] **Map**:
    - [ ] `{key: value,}` syntax (single pair with trailing comma).
    - [ ] `{key1: value1, key2: value2}` syntax (multiple pairs).
- [ ] **Specialized Types**:
  - [ ] **Color**:
    - [ ] Hexadecimal syntax `#RRGGBB`.
    - [ ] Function-call syntax `color(r, g, b)`.
  - [ ] **Coordinate**:
    - [ ] 2D `coord(x, y)`.
    - [ ] 3D `coord(x, y, z)`.
  - [ ] **Path**:
    - [ ] `path()` syntax.

## 4. Advanced Features

- [ ] **Quick Registration**:
  - [ ] `+= value` syntax within a section.
  - [ ] Automatically assigns an incrementing integer key (0, 1, 2, ...).
- [ ] **Arithmetic Operations**:
  - [ ] Supports `+`, `-`, `*`, `/`, `%`.
  - [ ] Supports `()` for priority.
  - [ ] Operations are limited to numeric types and macros resolving to numeric types.
- [ ] **Macros (`[#define]`)**:
  - [ ] Section `[#define]` for macro definitions.
  - [ ] `key = value` syntax for definitions.
  - [ ] `@name` syntax for referencing macros.
- [ ] **File Inclusion (`[#include]`)**:
  - [ ] `[#include]` section.
  - [ ] `+= path/to/file.yini` syntax.
  - [ ] Files are merged in order, with later files overriding earlier ones.
  - [ ] Merging applies to both regular sections and `[#define]` sections.
- [ ] **Environment Variables**:
  - [ ] `${VAR_NAME}` syntax for referencing environment variables.
- [ ] **Cross-Section References**:
  - [ ] `@{Section.key}` syntax for referencing a key in another section.

## 5. Schema Validation (`[#schema]`)

- [ ] **Declaration**:
  - [ ] `[#schema]` section containing section definitions to be validated.
  - [ ] `[SectionName]` within `[#schema]`.
  - [ ] `key = rule1, rule2, ...` syntax for rules.
- [ ] **Rule Types**:
  - [ ] **Requirement**: `!` (required), `?` (optional).
  - [ ] **Type Validation**:
    - [ ] `int`, `float`, `bool`, `string`, `array`, `list`, `map`, `color`, `coord`, `path`.
    - [ ] Array subtype syntax `array[int]`, `array[array[string]]`.
  - [ ] **Empty Behavior**:
    - [ ] `~` (ignore).
    - [ ] `e` (throw error).
    - [ ] `=default` (assign default value).
  - [ ] **Range Validation**:
    - [ ] `min=value`.
    - [ ] `max=value`.

## 6. Caching and Binary Format

- [ ] **YMETA Caching (`.ymeta`)**:
  - [ ] A `.ymeta` file is generated for each loaded `.yini` file to cache information.
- [ ] **Dyna Values**:
  - [ ] `Dyna(value)` or `dyna(value)` syntax.
  - [ ] Dynamic values are persisted in the `.ymeta` file.
  - [ ] `.ymeta` caches up to 5 previous values for `Dyna` keys.
- [ ] **YBIN Binary Asset (`.ybin`)**:
  - [ ] High-performance, zero-parsing format.
  - [ ] Contains fully resolved configuration data.

## 7. Command-Line Interface (CLI)

- [ ] **REPL (Interactive Mode)**:
  - [ ] Runs when `yini` is executed with no arguments.
  - [ ] Supports `.load <filepath>`.
  - [ ] Supports `.get <Section.key>`.
  - [ ] Supports `.exit`.
- [ ] **`cook` Command**:
  - [ ] `yini cook -o <output.ybin> <input.yini>...`
  - [ ] Creates a `.ybin` file from one or more `.yini` files.
- [ ] **`validate` Command**:
  - [ ] `yini validate <schema.yini> <config.yini>`
  - [ ] Validates a config file against a schema file.
- [ ] **`decompile` Command**:
  - [ ] `yini decompile <input.ybin>`
  - [ ] Converts a `.ybin` file back to a human-readable format.
- [ ] **Language Server**:
  - [ ] `yini --server` runs the CLI as a Language Server.

## 8. C# Bindings (`Yini.Core`)

- [ ] **Loading/Saving**:
  - [ ] `YiniConfig(string filePath)` constructor to load.
  - [ ] `YiniConfig()` constructor for in-memory creation.
  - [ ] `Save(string filePath)` method.
- [ ] **Getters**:
  - [ ] `GetInt`, `GetDouble`, `GetBool`, `GetString`.
  - [ ] Nullable return types (e.g., `int?`).
  - [ ] `Get...OrDefault` methods.
  - [ ] Generic `Get<T>` method.
  - [ ] Indexer support `config["key"]`.
- [ ] **Setters**:
  - [ ] `SetValue(string key, T value)` for primitive types.
- [ ] **Error Handling**:
  - [ ] Throws `YiniException` on failure.
- [ ] **Resource Management**:
  - [ ] Implements `IDisposable` to free native resources.

## 9. VSCode Extension (`vscode-yini`)

- [ ] **Activation**:
  - [ ] Activates for files with the `yini` language identifier.
- [ ] **Syntax Highlighting**:
  - [ ] Provides correct tokenization and coloring for all YINI syntax via `yini.tmLanguage.json`.
- [ ] **Language Server Client**:
  - [ ] Correctly starts and communicates with the `yini --server` process.
  - [ ] Provides diagnostics (errors/warnings) in the editor.
  - [ ] Provides hover information (e.g., type info).
  - [ ] Provides auto-completion (e.g., for macros).
  - [ ] Provides go-to-definition (e.g., for macros).
