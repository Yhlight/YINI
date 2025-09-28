using System;
using System.Runtime.InteropServices;

public class Program
{
    public static class NativeMethods
    {
        private const string DllName = "libyini.so";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr yini_load_from_string(string yini_string);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void yini_free_ast(IntPtr ast_ptr);
    }

    public static void Main()
    {
        Console.WriteLine("Testing YINI C# interop...");

        string yiniData = @"
[Networking]
host = ""127.0.0.1""
port = 8080
";

        IntPtr astPtr = IntPtr.Zero;
        try
        {
            astPtr = NativeMethods.yini_load_from_string(yiniData);
            if (astPtr != IntPtr.Zero)
            {
                Console.WriteLine("Successfully loaded YINI string into AST.");
            }
            else
            {
                Console.WriteLine("Failed to load YINI string.");
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"An error occurred: {e.Message}");
        }
        finally
        {
            if (astPtr != IntPtr.Zero)
            {
                NativeMethods.yini_free_ast(astPtr);
                Console.WriteLine("Successfully freed AST memory.");
            }
        }
    }
}