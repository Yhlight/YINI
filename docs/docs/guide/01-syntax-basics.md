---
sidebar_position: 1
title: Syntax Basics
---

# Syntax Basics

YINI builds upon the simple, human-readable structure of the classic INI format. This guide covers the fundamental components of the YINI language.

## File Structure

A YINI file is a plain text file, typically with a `.yini` extension. It is composed of sections, key-value pairs, and comments.

```yini
// This is a comment
[SectionName]
key = value
```

## Sections

Sections are used to group related key-value pairs. They act as namespaces for your configuration data.

-   **Declaration**: A section is declared by enclosing its name in square brackets `[]`.
-   **Naming**: Section names are typically `PascalCase` and can contain letters, numbers, and underscores.

```yini
[Core]
# This section contains core engine settings.

[Graphics]
# This section manages graphics settings.
```

## Key-Value Pairs

The core of a YINI file is the key-value pair, which assigns a value to a specific key within a section.

-   **Declaration**: A key-value pair is written as `key = value`.
-   **Keys**: Keys are identifiers, typically written in `snake_case`. They must be unique within their section.
-   **Values**: YINI supports a wide range of [typed values](./02-typed-values.md), from simple primitives to complex data structures.

```yini
[Player]
player_name = "Jules"
health = 100
is_invincible = false
```

## Comments

Comments are used to add explanatory notes to your configuration file. They are ignored by the parser. YINI supports two types of comments:

### Single-Line Comments

Use `//` or `#` to start a comment that lasts until the end of the line.

```yini
// This entire line is a comment.
key = value // This part is a comment.
# This is also a valid comment.
```

### Block Comments

Use `/* ... */` to create comments that can span multiple lines.

```yini
/*
  This is a multi-line
  block comment.
*/
key = value
```