using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Running;
using System.Collections.Generic;
using Yini;

[MemoryDiagnoser]
public class GetBenchmarks
{
    private YiniManager _manager = new();

    [GlobalSetup]
    public void GlobalSetup()
    {
        const string content = @"
[Data]
string_val = ""This is a test string for benchmarking purposes.""
int_val = 12345
double_val = 987.654
bool_val = true
list_val = [""a"", ""b"", ""c"", ""d"", ""e""]
map_val = { ""key1"": 1, ""key2"": 2, ""key3"": 3 }
";
        _manager.LoadFromString(content);
    }

    [GlobalCleanup]
    public void GlobalCleanup()
    {
        _manager.Dispose();
    }

    [Benchmark(Description = "Get<string>")]
    public string GetString()
    {
        return _manager.Get<string>("Data", "string_val")!;
    }

    [Benchmark(Description = "Get<int>")]
    public int GetInt()
    {
        return _manager.Get<int>("Data", "int_val");
    }

    [Benchmark(Description = "Get<double>")]
    public double GetDouble()
    {
        return _manager.Get<double>("Data", "double_val");
    }

    [Benchmark(Description = "Get<bool>")]
    public bool GetBool()
    {
        return _manager.Get<bool>("Data", "bool_val");
    }

    [Benchmark(Description = "Get<List<string>>")]
    public List<string> GetList()
    {
        return _manager.Get<List<string>>("Data", "list_val")!;
    }

    [Benchmark(Description = "Get<Dictionary<string, int>>")]
    public Dictionary<string, int> GetDictionary()
    {
        return _manager.Get<Dictionary<string, int>>("Data", "map_val")!;
    }
}

public class Program
{
    public static void Main(string[] args)
    {
        Console.WriteLine("Running C# Benchmarks...");
        var summary = BenchmarkRunner.Run<GetBenchmarks>();
        Console.WriteLine(summary);
    }
}