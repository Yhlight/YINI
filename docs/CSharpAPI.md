# YINI C# API Reference

This document provides a reference for the YINI C# API.

## YiniManager Class

The `YiniManager` class is the main entry point for interacting with the YINI library.

### Constructor

**`YiniManager()`**

Creates a new instance of the `YiniManager` class.

### Methods

**`bool Load(string filepath)`**

Loads a YINI file from the specified path.

**`void SaveChanges()`**

Saves any changes made to the YINI file.

**`double GetDouble(string section, string key, double defaultValue = 0.0)`**

Gets a double value from the specified section and key.

**`string GetString(string section, string key, string defaultValue = "")`**

Gets a string value from the specified section and key.

**`bool GetBool(string section, string key, bool defaultValue = false)`**

Gets a boolean value from the specified section and key.

**`void SetDouble(string section, string key, double value)`**

Sets a double value for the specified section and key.

**`void SetString(string section, string key, string value)`**

Sets a string value for the specified section and key.

**`void SetBool(string section, string key, bool value)`**

Sets a boolean value for the specified section and key.

**`void Dispose()`**

Disposes of the `YiniManager` instance and releases any unmanaged resources.