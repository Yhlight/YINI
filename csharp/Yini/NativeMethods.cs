using System;
using System.Runtime.InteropServices;
using System.Security;
using Microsoft.Win32.SafeHandles;

namespace YINI
{
    [SecurityCritical]
    internal static class NativeMethods
    {
        private const string LibName = "yini";

        // Document API
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern YiniDocumentHandle yini_parse(string content, IntPtr error_buffer, int buffer_size);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void yini_free_document(IntPtr handle);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int yini_get_section_count(YiniDocumentHandle handle);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr yini_get_section_by_name(YiniDocumentHandle handle, string name);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr yini_section_get_value_by_key(IntPtr sectionHandle, string key);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern void yini_set_string_value(YiniDocumentHandle handle, string section, string key, string value);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern void yini_set_int_value(YiniDocumentHandle handle, string section, string key, int value);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern void yini_set_double_value(YiniDocumentHandle handle, string section, string key, double value);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern void yini_set_bool_value(YiniDocumentHandle handle, string section, string key, bool value);

    }

    internal class YiniDocumentHandle : SafeHandleZeroOrMinusOneIsInvalid
    {
        private YiniDocumentHandle() : base(true) {}

        protected override bool ReleaseHandle()
        {
            NativeMethods.yini_free_document(handle);
            return true;
        }
    }
}