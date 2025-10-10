using System;
using System.Runtime.InteropServices;

namespace Yini.Core
{
    internal static class NativeMethods
    {
        private const string LibName = "YiniInterop";

        [DllImport(LibName, EntryPoint = "yini_create_from_file", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr YiniCreateFromFile(string filePath);

        [DllImport(LibName, EntryPoint = "yini_destroy", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniDestroy(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_int", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetInt(IntPtr handle, string key, out int outValue);

        [DllImport(LibName, EntryPoint = "yini_get_double", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetDouble(IntPtr handle, string key, out double outValue);

        [DllImport(LibName, EntryPoint = "yini_get_bool", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetBool(IntPtr handle, string key, out bool outValue);

        [DllImport(LibName, EntryPoint = "yini_get_string", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetString(IntPtr handle, string key);

        [DllImport(LibName, EntryPoint = "yini_free_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniFreeString(IntPtr str);

        public static string GetString(IntPtr handle, string key)
        {
            IntPtr cstr = YiniGetString(handle, key);
            if (cstr == IntPtr.Zero)
            {
                return null;
            }
            try
            {
                return Marshal.PtrToStringAnsi(cstr);
            }
            finally
            {
                YiniFreeString(cstr);
            }
        }
    }

    public class YiniConfig : IDisposable
    {
        private IntPtr m_handle;
        private bool m_disposed = false;

        public YiniConfig(string filePath)
        {
            m_handle = NativeMethods.YiniCreateFromFile(filePath);
            if (m_handle == IntPtr.Zero)
            {
                throw new Exception("Failed to create YINI config.");
            }
        }

        public bool GetInt(string key, out int value)
        {
            return NativeMethods.YiniGetInt(m_handle, key, out value);
        }

        public bool GetDouble(string key, out double value)
        {
            return NativeMethods.YiniGetDouble(m_handle, key, out value);
        }

        public bool GetBool(string key, out bool value)
        {
            return NativeMethods.YiniGetBool(m_handle, key, out value);
        }

        public string GetString(string key)
        {
            return NativeMethods.GetString(m_handle, key);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!m_disposed)
            {
                if (m_handle != IntPtr.Zero)
                {
                    NativeMethods.YiniDestroy(m_handle);
                    m_handle = IntPtr.Zero;
                }
                m_disposed = true;
            }
        }

        ~YiniConfig()
        {
            Dispose(false);
        }
    }
}