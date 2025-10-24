# YINI `Dyna()` API Design

## 1. Overview

This document proposes a design for the `Dyna()` dynamic value API, intended to provide a powerful and flexible system for managing dynamic configuration in game development. The API is designed with performance, ease of use, and interoperability between C++ and C# in mind.

## 2. Core Concepts

*   **`DynaValue<T>`:** A generic class that represents a dynamic value of a specific type. It provides methods for getting and setting the value, as well as for subscribing to change events.
*   **`DynaContext`:** A class that manages a collection of `DynaValue` objects. It provides methods for creating, retrieving, and updating dynamic values, as well as for managing their persistence to `.ymeta` files.
*   **Transactions:** A mechanism for atomically updating multiple `DynaValue` objects. This ensures that a set of related changes are applied together, and that change events are fired only after all changes have been made.
*   **Scoping:** `DynaContext` objects can be nested to create a hierarchy of dynamic values. This allows for different scopes of configuration, such as global, per-level, or per-savegame.

## 3. C++ API

```cpp
#include <functional>
#include <string>
#include <vector>

namespace YINI
{

template<typename T>
class DynaValue
{
public:
    // Get the current value.
    const T& get() const;

    // Set a new value.
    void set(const T& value);

    // Subscribe to change events.
    using ChangeCallback = std::function<void(const T& newValue)>;
    int subscribe(ChangeCallback callback);

    // Unsubscribe from change events.
    void unsubscribe(int subscriptionId);
};

class DynaContext
{
public:
    // Create a new DynaContext.
    DynaContext(const std::string& ymetaPath);

    // Get or create a DynaValue.
    template<typename T>
    DynaValue<T>& get(const std::string& key, const T& defaultValue);

    // Begin a transaction.
    void beginTransaction();

    // End a transaction.
    void endTransaction();

    // Save the context to the .ymeta file.
    void save();
};

} // namespace YINI
```

### Example Usage (C++)

```cpp
#include "Yini/Dyna.h"

int main()
{
    // Create a global context.
    YINI::DynaContext globalContext("global.ymeta");

    // Get a dynamic value for the player's health.
    auto& playerHealth = globalContext.get<int>("player.health", 100);

    // Subscribe to health changes.
    int subscriptionId = playerHealth.subscribe([](int newHealth) {
        // Update the UI.
    });

    // Update the player's health and mana in a transaction.
    globalContext.beginTransaction();
    playerHealth.set(90);
    globalContext.get<int>("player.mana", 50).set(45);
    globalContext.endTransaction();

    // Save the changes.
    globalContext.save();

    return 0;
}
```

## 4. C# API

```csharp
using System;

namespace Yini.Core
{
    public class DynaValue<T>
    {
        // Get the current value.
        public T Value { get; set; }

        // Subscribe to change events.
        public event Action<T> OnChanged;
    }

    public class DynaContext : IDisposable
    {
        // Create a new DynaContext.
        public DynaContext(string ymetaPath);

        // Get or create a DynaValue.
        public DynaValue<T> Get<T>(string key, T defaultValue);

        // Begin a transaction.
        public void BeginTransaction();

        // End a transaction.
        public void EndTransaction();

        // Save the context to the .ymeta file.
        public void Save();

        // Dispose of the context.
        public void Dispose();
    }
}
```

### Example Usage (C#)

```csharp
using Yini.Core;

public class Player
{
    private DynaContext _context;
    private DynaValue<int> _health;

    public Player()
    {
        _context = new DynaContext("player.ymeta");
        _health = _context.Get<int>("player.health", 100);
        _health.OnChanged += OnHealthChanged;
    }

    private void OnHealthChanged(int newHealth)
    {
        // Update the UI.
    }

    public void TakeDamage(int amount)
    {
        _context.BeginTransaction();
        _health.Value -= amount;
        _context.EndTransaction();
        _context.Save();
    }
}
```
