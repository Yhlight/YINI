using System;
using YINI;

class Example
{
    static void Main(string[] args)
    {
        // Example 1: Parse YINI from string
        Console.WriteLine("=== Example 1: Parse from string ===\n");
        
        string source = @"
[Server]
host = ""localhost""
port = 8080
debug = true

[Database]
connection = ""mongodb://localhost:27017""
pool_size = 10
";

        using (var parser = new Parser(source))
        {
            if (parser.Parse())
            {
                Console.WriteLine("✓ Parse successful!");
                Console.WriteLine($"Sections: {parser.GetSectionCount()}");
                
                foreach (var name in parser.GetSectionNames())
                {
                    Console.WriteLine($"\n[{name}]");
                    var section = parser.GetSection(name);
                    if (section != null)
                    {
                        foreach (var key in section.GetKeys())
                        {
                            var value = section.GetValue(key);
                            if (value != null)
                            {
                                switch (value.GetValueType())
                                {
                                    case ValueType.Integer:
                                        Console.WriteLine($"  {key} = {value.AsInteger()}");
                                        break;
                                    case ValueType.Boolean:
                                        Console.WriteLine($"  {key} = {value.AsBoolean()}");
                                        break;
                                    case ValueType.String:
                                        Console.WriteLine($"  {key} = \"{value.AsString()}\"");
                                        break;
                                    default:
                                        Console.WriteLine($"  {key} = <{value.GetValueType()}>");
                                        break;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                Console.WriteLine($"✗ Parse failed: {parser.GetError()}");
            }
        }

        // Example 2: Parse from file
        Console.WriteLine("\n\n=== Example 2: Parse from file ===\n");
        
        try
        {
            using (var parser = Parser.FromFile("../examples/simple.yini"))
            {
                if (parser.Parse())
                {
                    Console.WriteLine("✓ File parsed successfully!");
                    
                    var server = parser.GetSection("Server");
                    if (server != null)
                    {
                        var host = server.GetValue("host");
                        var port = server.GetValue("port");
                        
                        if (host != null && port != null)
                        {
                            Console.WriteLine($"Server: {host.AsString()}:{port.AsInteger()}");
                        }
                    }
                }
                else
                {
                    Console.WriteLine($"✗ Parse failed: {parser.GetError()}");
                }
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"Error: {e.Message}");
        }

        // Example 3: Compile to YMETA
        Console.WriteLine("\n\n=== Example 3: Compile to YMETA ===\n");
        
        if (Parser.CompileToYMETA("../examples/simple.yini", "test.ymeta"))
        {
            Console.WriteLine("✓ Compiled to test.ymeta");
            
            if (Parser.DecompileFromYMETA("test.ymeta", "test_output.yini"))
            {
                Console.WriteLine("✓ Decompiled to test_output.yini");
            }
        }
        else
        {
            Console.WriteLine("✗ Compilation failed");
        }

        Console.WriteLine("\n=== Done! ===");
    }
}
