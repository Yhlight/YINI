using System;
using System.Runtime.InteropServices;
using System.Text;

public static class Yini
{
    // Define the name of the native library.
    // On Linux, this is libyini_core.so. The 'lib' prefix and '.so' suffix are added automatically.
    private const string NativeLib = "yini_core";

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_load_from_string")]
    public static extern IntPtr LoadFromString(string content);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_free")]
    public static extern void Free(IntPtr handle);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_integer")]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool GetInteger(IntPtr handle, string section, string key, out long outValue);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_float")]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool GetFloat(IntPtr handle, string section, string key, out double outValue);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_bool")]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool GetBool(IntPtr handle, string section, string key, [MarshalAs(UnmanagedType.I1)] out bool outValue);

    [DllImport(NativeLib, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_string")]
    public static extern int GetString(IntPtr handle, string section, string key, StringBuilder outBuffer, int bufferSize);
}

public class YiniExample
{
    public static void Main(string[] args)
    {
        Console.WriteLine("--- C# P/Invoke YINI Test ---");

        string yiniContent = @"
[Credentials]
user = ""Jules""
token = ""abc-123""

[Config]
version = 1.2
retries = 5
enabled = true
";

        IntPtr handle = Yini.LoadFromString(yiniContent);
        if (handle == IntPtr.Zero)
        {
            Console.WriteLine("Failed to load YINI content.");
            return;
        }

        Console.WriteLine("YINI handle loaded successfully.");

        // Test String
        StringBuilder buffer = new StringBuilder(100);
        if (Yini.GetString(handle, "Credentials", "user", buffer, buffer.Capacity) > 0)
        {
            Console.WriteLine($"SUCCESS: Credentials.user = {buffer}");
        }
        else
        {
            Console.WriteLine("FAILURE: Could not get Credentials.user");
        }

        // Test Integer
        if (Yini.GetInteger(handle, "Config", "retries", out long retries))
        {
            Console.WriteLine($"SUCCESS: Config.retries = {retries}");
        }
        else
        {
            Console.WriteLine("FAILURE: Could not get Config.retries");
        }

        // Test Float
        if (Yini.GetFloat(handle, "Config", "version", out double version))
        {
            Console.WriteLine($"SUCCESS: Config.version = {version}");
        }
        else
        {
            Console.WriteLine("FAILURE: Could not get Config.version");
        }

        // Test Boolean
        if (Yini.GetBool(handle, "Config", "enabled", out bool enabled))
        {
            Console.WriteLine($"SUCCESS: Config.enabled = {enabled}");
        }
        else
        {
            Console.WriteLine("FAILURE: Could not get Config.enabled");
        }

        // Free the native resource
        Yini.Free(handle);
        Console.WriteLine("YINI handle freed.");

        Console.WriteLine("--- End C# Test ---");
    }
}
