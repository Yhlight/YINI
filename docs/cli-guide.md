# YINI CLI Guide

The `yini-cli` is a command-line tool for managing and validating YINI files.

## 1. Commands

### `check`

Validates the syntax and integrity of a `.yini` file, including all its includes and inheritance chains.

**Usage:**
```bash
yini-cli check <filepath>
```

**Example:**
```bash
yini-cli check config.yini
```
If the file is valid, it will print a success message. Otherwise, it will print a detailed error message.

### `compile`

Compiles a `.yini` file into a binary `.ymeta` format. This format is optimized for fast loading at runtime, as it contains the fully resolved configuration state.

**Usage:**
```bash
yini-cli compile <input.yini> <output.ymeta>
```

**Example:**
```bash
yini-cli compile config.yini config.ymeta
```

### `decompile`

Reads a binary `.ymeta` file and prints its contents to the console in a human-readable format. This is useful for inspecting the contents of a compiled configuration.

**Usage:**
```bash
yini-cli decompile <filepath.ymeta>
```

**Example:**
```bash
yini-cli decompile config.ymeta
```