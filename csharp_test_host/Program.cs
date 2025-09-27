using System;
using System.Runtime.InteropServices;

public class Program
{
    // On Linux, the library is called libyini_core.so
    // On Windows, it would be yini_core.dll
    // DllImport will handle the platform-specific name resolution.
    private const string NativeLibName = "yini_core";

    [DllImport(NativeLibName, EntryPoint = "add")]
    public static extern int Add(int a, int b);

    public static void Main(string[] args)
    {
        Console.WriteLine("Attempting to call native C++ function...");
        int result = Add(5, 7);
        Console.WriteLine($"Result from C++ add(5, 7): {result}");

        if (result == 12)
        {
            Console.WriteLine("P/Invoke test SUCCEEDED!");
        }
        else
        {
            Console.WriteLine($"P/Invoke test FAILED! Expected 12 but got {result}");
        }
    }
}