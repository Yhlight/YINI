using System;
using System.Runtime.InteropServices;

public static class Yini
{
    private const string LibName = "yini";

    // --- Enums and Handles ---

    // This must be kept in sync with the C++ YiniResult enum
    public enum Result
    {
        OK = 0,
        ErrorFileNotFound,
        ErrorParseError,
        ErrorKeyNotFound,
        ErrorTypeMismatch,
        ErrorEvaluationError,
        ErrorUnknown
    }

    // The C# representation of the opaque handle.
    // It's a "safe" handle, meaning the runtime knows how to release it,
    // preventing memory leaks if the C# consumer forgets to call Free.
    public class DocumentHandle : SafeHandle
    {
        public DocumentHandle() : base(IntPtr.Zero, true) {}

        public override bool IsInvalid => handle == IntPtr.Zero;

        protected override bool ReleaseHandle()
        {
            // This calls our C Yini_Free function when the handle is disposed
            Yini_Free(handle);
            return true;
        }
    }

    // --- P/Invoke Declarations ---

    [DllImport(LibName, EntryPoint = "Yini_LoadFromFile", CallingConvention = CallingConvention.Cdecl)]
    public static extern Result LoadFromFile(string filepath, out DocumentHandle handle);

    [DllImport(LibName, EntryPoint = "Yini_Free", CallingConvention = CallingConvention.Cdecl)]
    private static extern void Yini_Free(IntPtr handle);

    [DllImport(LibName, EntryPoint = "Yini_GetValue_Int", CallingConvention = CallingConvention.Cdecl)]
    public static extern Result GetValue(DocumentHandle handle, string key, out long value);
}

/*
// --- EXAMPLE USAGE ---

public class YiniExample
{
    public static void Main(string[] args)
    {
        string filePath = "path/to/your/file.yini";

        Yini.Result result = Yini.LoadFromFile(filePath, out Yini.DocumentHandle doc);

        if (result != Yini.Result.OK)
        {
            Console.WriteLine($"Error loading YINI file: {result}");
            return;
        }

        using (doc) // using statement ensures the handle is disposed and Yini_Free is called
        {
            Console.WriteLine("YINI file loaded successfully.");

            // Example: Get an integer value
            result = Yini.GetValue(doc, "PlayerStats.attack", out long attackValue);
            if (result == Yini.Result.OK)
            {
                Console.WriteLine($"Player attack value is: {attackValue}");
            }
            else
            {
                Console.WriteLine($"Failed to get 'PlayerStats.attack': {result}");
            }
        }
    }
}
*/
