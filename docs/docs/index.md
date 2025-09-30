---
sidebar_position: 1
---

# Welcome to YINI

YINI is a powerful and feature-rich configuration file format designed to be both easy to read and highly functional for complex applications like games.

## What is YINI?

At its core, YINI is an extension of the classic INI file format. It maintains the simple `key = value` structure within `[sections]`, but adds a host of modern features, including:

- **Section Inheritance**: Reduce duplication by inheriting values from base sections.
- **Typed Values**: Native support for strings, integers, booleans, floats, arrays, maps, and more.
- **Macros**: Define constants in a `[#define]` block and reuse them throughout your files.
- **File Includes**: Split your configuration across multiple files and include them where needed.
- **Dynamic Values**: Mark values with `Dyna()` to have them automatically updated and written back to the source file.

## Getting Started

This documentation will guide you through the syntax of the YINI language, how to use the C API to parse files in your own C++ projects, and how to use the provided command-line tools.

Navigate through the sidebar to explore the features of YINI.