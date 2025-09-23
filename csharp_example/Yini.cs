using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Yini
{
    /// <summary>
    /// A static wrapper class for the native YINI C API using P/Invoke.
    /// </summary>
    public static class Native
    {
        private const string LibName = "yini";

        /// <summary>
        /// Loads and processes a YINI file from the given path.
        /// </summary>
        /// <param name="filepath">The path to the .yini file.</param>
        /// <returns>A handle to the loaded YINI data, or IntPtr.Zero if loading fails.</returns>
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_load")]
        public static extern IntPtr Load(string filepath);

        /// <summary>
        /// Frees the memory associated with a YiniHandle.
        /// </summary>
        /// <param name="handle">The handle returned by Load().</param>
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_free")]
        public static extern void Free(IntPtr handle);

        /// <summary>
        /// Retrieves a string value from the YINI data.
        /// </summary>
        /// <param name="handle">The handle to the YINI data.</param>
        /// <param name="section">The name of the section.</param>
        /// <param name="key">The name of the key.</param>
        /// <param name="outBuffer">The buffer to write the string into.</param>
        /// <param name="bufferSize">The size of the output buffer.</param>
        /// <returns>The number of bytes written, or -1 on failure.</returns>
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_string")]
        public static extern int GetString(IntPtr handle, string section, string key, StringBuilder outBuffer, int bufferSize);

        /// <summary>
        /// Retrieves a 64-bit integer value.
        /// </summary>
        /// <param name="outValue">A reference to store the retrieved integer.</param>
        /// <returns>1 on success, 0 on failure.</returns>
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_int64")]
        public static extern int GetInt64(IntPtr handle, string section, string key, out long outValue);

        /// <summary>
        /// Retrieves a double-precision float value.
        /// </summary>
        /// <param name="outValue">A reference to store the retrieved double.</param>
        /// <returns>1 on success, 0 on failure.</returns>
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_double")]
        public static extern int GetDouble(IntPtr handle, string section, string key, out double outValue);

        /// <summary>
        /// Retrieves a boolean value.
        /// </summary>
        /// <param name="outValue">A reference to store the retrieved boolean.</param>
        /// <returns>1 on success, 0 on failure.</returns>
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_bool")]
        public static extern int GetBool(IntPtr handle, string section, string key, out bool outValue);
    }
}
