# YINI C# Bindings

C# bindings for the YINI configuration language using P/Invoke.

## Prerequisites

- .NET Core 3.1+ or .NET Framework 4.7.2+
- YINI shared library (`libyini.so` on Linux, `yini.dll` on Windows)

## Installation

1. Copy `YINI.cs` to your C# project
2. Ensure `libyini.so` (or `yini.dll`) is in your library path:
   - Linux: Copy to `/usr/local/lib` or add to `LD_LIBRARY_PATH`
   - Windows: Copy to same directory as executable or system PATH
   - macOS: Copy to `/usr/local/lib` or add to `DYLD_LIBRARY_PATH`

## Usage

### Basic Parsing

```csharp
using YINI;

string source = @"
[Config]
width = 1920
height = 1080
fullscreen = true
title = ""My Game""
";

using (var parser = new Parser(source))
{
    if (parser.Parse())
    {
        var config = parser.GetSection("Config");
        if (config != null)
        {
            var width = config.GetValue("width")?.AsInteger();
            var height = config.GetValue("height")?.AsInteger();
            var fullscreen = config.GetValue("fullscreen")?.AsBoolean();
            var title = config.GetValue("title")?.AsString();
            
            Console.WriteLine($"{width}x{height}, Fullscreen: {fullscreen}");
            Console.WriteLine($"Title: {title}");
        }
    }
    else
    {
        Console.WriteLine($"Error: {parser.GetError()}");
    }
}
```

### Parse from File

```csharp
using (var parser = Parser.FromFile("config.yini"))
{
    if (parser.Parse())
    {
        foreach (var sectionName in parser.GetSectionNames())
        {
            Console.WriteLine($"[{sectionName}]");
            
            var section = parser.GetSection(sectionName);
            if (section != null)
            {
                foreach (var key in section.GetKeys())
                {
                    var value = section.GetValue(key);
                    Console.WriteLine($"  {key} = {value?.AsString()}");
                }
            }
        }
    }
}
```

### Working with Arrays

```csharp
var items = section.GetValue("items");
if (items != null && items.GetValueType() == ValueType.Array)
{
    int size = items.GetArraySize();
    for (int i = 0; i < size; i++)
    {
        var element = items.GetArrayElement(i);
        Console.WriteLine(element?.AsString());
    }
}
```

### Compile to YMETA

```csharp
// Compile YINI to binary format
if (Parser.CompileToYMETA("config.yini", "config.ymeta"))
{
    Console.WriteLine("Compiled successfully!");
}

// Decompile back to text
if (Parser.DecompileFromYMETA("config.ymeta", "config_restored.yini"))
{
    Console.WriteLine("Decompiled successfully!");
}
```

## API Reference

### Parser Class

- `Parser(string source)` - Create parser from YINI source string
- `static Parser FromFile(string filename)` - Create parser from file
- `bool Parse()` - Parse the YINI source
- `string GetError()` - Get last error message
- `int GetSectionCount()` - Get number of sections
- `string[] GetSectionNames()` - Get all section names
- `Section? GetSection(string name)` - Get section by name
- `static bool CompileToYMETA(string inputFile, string outputFile)` - Compile to YMETA
- `static bool DecompileFromYMETA(string inputFile, string outputFile)` - Decompile from YMETA

### Section Class

- `Value? GetValue(string key)` - Get value by key
- `string[] GetKeys()` - Get all keys in section

### Value Class

- `ValueType GetValueType()` - Get value type
- `long AsInteger()` - Get as integer
- `double AsFloat()` - Get as float
- `bool AsBoolean()` - Get as boolean
- `string AsString()` - Get as string
- `int GetArraySize()` - Get array size
- `Value? GetArrayElement(int index)` - Get array element

### ValueType Enum

- `Nil` - Null value
- `Integer` - 64-bit integer
- `Float` - Double precision float
- `Boolean` - Boolean value
- `String` - String value
- `Array` - Array type
- `Map` - Map/dictionary type
- `Color` - Color value
- `Coord` - Coordinate value

## Building the Example

### Quick Build (using build script)

```bash
# Build YINI library first
cd ../..
./build.py --clean

# Build C# bindings
cd bindings/csharp
./build_csharp.sh
```

The `build_csharp.sh` script will automatically:
- Check for Mono or .NET SDK
- Verify the YINI library is built
- Compile the C# bindings
- Provide run instructions

### Manual Build

```bash
# On Linux/macOS with Mono
cd bindings/csharp
mcs Example.cs YINI.cs -out:example.exe
LD_LIBRARY_PATH=../../build/lib mono example.exe

# On Windows
csc Example.cs YINI.cs
copy ..\..\build\lib\yini.dll .
example.exe
```

## Notes

- The Parser class implements IDisposable - always use `using` statement or call `Dispose()`
- String and array handles are only valid while the Parser object is alive
- Thread safety: Create separate Parser instances for different threads

## License

Same as YINI project
