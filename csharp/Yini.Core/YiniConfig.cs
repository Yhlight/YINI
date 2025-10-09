using System;
using System.Runtime.InteropServices;

namespace Yini.Core
{
    public class YiniConfig : IDisposable
    {
        private const string LibName = "YiniInterop";

        private IntPtr _handle;
        private bool _disposed = false;

        [DllImport(LibName, EntryPoint = "yini_parse_file", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniParseFile(string filepath);

        [DllImport(LibName, EntryPoint = "yini_get_string", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetStringInternal(IntPtr handle, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_get_int", CallingConvention = CallingConvention.Cdecl)]
        private static extern int YiniGetInt(IntPtr handle, string section, string key, int defaultValue);

        [DllImport(LibName, EntryPoint = "yini_get_double", CallingConvention = CallingConvention.Cdecl)]
        private static extern double YiniGetDouble(IntPtr handle, string section, string key, double defaultValue);

        [DllImport(LibName, EntryPoint = "yini_get_bool", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool YiniGetBool(IntPtr handle, string section, string key, bool defaultValue);

        [DllImport(LibName, EntryPoint = "yini_free_config", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniFreeConfig(IntPtr handle);

        public YiniConfig(string filepath)
        {
            _handle = YiniParseFile(filepath);
            if (_handle == IntPtr.Zero)
            {
                throw new Exception($"Failed to parse YINI file: {filepath}");
            }
        }

        public string GetString(string section, string key, string defaultValue = null)
        {
            IntPtr cstr = YiniGetStringInternal(_handle, section, key);
            if (cstr == IntPtr.Zero)
            {
                return defaultValue;
            }
            return Marshal.PtrToStringAnsi(cstr);
        }

        public int GetInt(string section, string key, int defaultValue = 0)
        {
            return YiniGetInt(_handle, section, key, defaultValue);
        }

        public double GetDouble(string section, string key, double defaultValue = 0.0)
        {
            return YiniGetDouble(_handle, section, key, defaultValue);
        }

        public bool GetBool(string section, string key, bool defaultValue = false)
        {
            return YiniGetBool(_handle, section, key, defaultValue);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                {
                    YiniFreeConfig(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~YiniConfig()
        {
            Dispose(false);
        }
    }
}