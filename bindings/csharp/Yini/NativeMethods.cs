using System;
using System.Runtime.InteropServices;

namespace Yini
{
    internal static class NativeMethods
    {
        private const string LibName = "yini_shared";

        [DllImport(LibName, EntryPoint = "yini_parse_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr YiniParseString(string source);

        [DllImport(LibName, EntryPoint = "yini_free_document", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniFreeDocument(IntPtr handle);
    }
}