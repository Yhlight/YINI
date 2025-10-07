using System;
using System.Runtime.InteropServices;

namespace Yini.Bindings
{
    public class Yini : IDisposable
    {
        private IntPtr handle;
        private bool disposed = false;

        private const string LibName = "yini_c";

        [DllImport(LibName, EntryPoint = "yini_load")]
        private static extern IntPtr YiniLoad(string filepath);

        [DllImport(LibName, EntryPoint = "yini_free")]
        private static extern void YiniFree(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_string", CharSet = CharSet.Ansi)]
        private static extern IntPtr YiniGetString(IntPtr handle, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_free_string")]
        private static extern void YiniFreeString(IntPtr str);

        [DllImport(LibName, EntryPoint = "yini_get_int")]
        private static extern int YiniGetInt(IntPtr handle, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_get_double")]
        private static extern double YiniGetDouble(IntPtr handle, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_get_bool")]
        private static extern bool YiniGetBool(IntPtr handle, string section, string key);

        public Yini(string filepath)
        {
            handle = YiniLoad(filepath);
            if (handle == IntPtr.Zero)
            {
                throw new Exception($"Failed to load YINI file: {filepath}");
            }
        }

        public string GetString(string section, string key)
        {
            IntPtr cstr = YiniGetString(handle, section, key);
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

        public int GetInt(string section, string key)
        {
            return YiniGetInt(handle, section, key);
        }

        public double GetDouble(string section, string key)
        {
            return YiniGetDouble(handle, section, key);
        }

        public bool GetBool(string section, string key)
        {
            return YiniGetBool(handle, section, key);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (handle != IntPtr.Zero)
                {
                    YiniFree(handle);
                    handle = IntPtr.Zero;
                }
                disposed = true;
            }
        }

        ~Yini()
        {
            Dispose(false);
        }
    }
}