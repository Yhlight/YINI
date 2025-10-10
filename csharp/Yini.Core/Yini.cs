using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Collections.Generic;

namespace Yini.Core
{
    // --- Structs for complex types ---
    [StructLayout(LayoutKind.Sequential)]
    public struct YiniColor
    {
        public byte r, g, b;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct YiniCoord
    {
        public double x, y, z;
        [MarshalAs(UnmanagedType.I1)]
        public bool has_z;
    }

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

        [DllImport(LibName, EntryPoint = "yini_get_string_length", CallingConvention = CallingConvention.Cdecl)]
        public static extern int YiniGetStringLength(IntPtr handle, string key);

        [DllImport(LibName, EntryPoint = "yini_get_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern int YiniGetString(IntPtr handle, string key, StringBuilder buffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_get_color", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetColor(IntPtr handle, string key, out YiniColor outValue);

        [DllImport(LibName, EntryPoint = "yini_get_coord", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetCoord(IntPtr handle, string key, out YiniCoord outValue);

        [DllImport(LibName, EntryPoint = "yini_get_array_size", CallingConvention = CallingConvention.Cdecl)]
        public static extern int YiniGetArraySize(IntPtr handle, string key);

        [DllImport(LibName, EntryPoint = "yini_get_array_item_as_string_length", CallingConvention = CallingConvention.Cdecl)]
        public static extern int YiniGetArrayItemAsStringLength(IntPtr handle, string key, int index);

        [DllImport(LibName, EntryPoint = "yini_get_array_item_as_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern int YiniGetArrayItemAsString(IntPtr handle, string key, int index, StringBuilder buffer, int bufferSize);

        public static string GetString(IntPtr handle, string key)
        {
            int length = YiniGetStringLength(handle, key);
            if (length < 0) return null;
            StringBuilder buffer = new StringBuilder(length + 1);
            YiniGetString(handle, key, buffer, buffer.Capacity);
            return buffer.ToString();
        }

        public static string GetArrayItemAsString(IntPtr handle, string key, int index)
        {
            int length = YiniGetArrayItemAsStringLength(handle, key, index);
            if (length < 0) return null;
            StringBuilder buffer = new StringBuilder(length + 1);
            YiniGetArrayItemAsString(handle, key, index, buffer, buffer.Capacity);
            return buffer.ToString();
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

        public bool GetColor(string key, out YiniColor value)
        {
            return NativeMethods.YiniGetColor(m_handle, key, out value);
        }

        public bool GetCoord(string key, out YiniCoord value)
        {
            return NativeMethods.YiniGetCoord(m_handle, key, out value);
        }

        public string[] GetStringArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0)
            {
                return null;
            }
            var result = new string[size];
            for (int i = 0; i < size; i++)
            {
                result[i] = NativeMethods.GetArrayItemAsString(m_handle, key, i);
            }
            return result;
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