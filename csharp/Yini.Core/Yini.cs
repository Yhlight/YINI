using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace Yini.Core
{
    internal static class NativeMethods
    {
        private const string LibName = "YiniInterop";

        [DllImport(LibName, EntryPoint = "yini_create_from_file", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr YiniCreateFromFile(string filePath);

        [DllImport(LibName, EntryPoint = "yini_get_last_error", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetLastError();

        public static string GetLastError()
        {
            IntPtr cstr = YiniGetLastError();
            return cstr == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(cstr);
        }

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

        [DllImport(LibName, EntryPoint = "yini_get_array_size", CallingConvention = CallingConvention.Cdecl)]
        public static extern int YiniGetArraySize(IntPtr handle, string key);

        [DllImport(LibName, EntryPoint = "yini_get_array_item_as_int", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetArrayItemAsInt(IntPtr handle, string key, int index, out int outValue);

        [DllImport(LibName, EntryPoint = "yini_get_array_item_as_double", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetArrayItemAsDouble(IntPtr handle, string key, int index, out double outValue);

        [DllImport(LibName, EntryPoint = "yini_get_array_item_as_bool", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetArrayItemAsBool(IntPtr handle, string key, int index, out bool outValue);

        [DllImport(LibName, EntryPoint = "yini_get_array_item_as_string", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetArrayItemAsString(IntPtr handle, string key, int index);

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

        public static string GetArrayItemAsString(IntPtr handle, string key, int index)
        {
            IntPtr cstr = YiniGetArrayItemAsString(handle, key, index);
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

    /// <summary>
    /// Represents errors that occur during YINI configuration processing.
    /// </summary>
    public class YiniException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="YiniException"/> class with a specified error message.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        public YiniException(string message) : base(message) { }
    }

    /// <summary>
    /// Provides a managed interface to a YINI configuration file.
    /// This class handles the lifetime of the native YINI handle and provides methods to access configuration values.
    /// </summary>
    public class YiniConfig : IDisposable
    {
        private IntPtr m_handle;
        private bool m_disposed = false;

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniConfig"/> class by loading and parsing a YINI file.
        /// </summary>
        /// <param name="filePath">The path to the .yini file.</param>
        /// <exception cref="YiniException">Thrown if the native library fails to load or parse the file.</exception>
        public YiniConfig(string filePath)
        {
            m_handle = NativeMethods.YiniCreateFromFile(filePath);
            if (m_handle == IntPtr.Zero)
            {
                string errorMessage = NativeMethods.GetLastError();
                throw new YiniException($"Failed to create YINI config: {errorMessage}");
            }
        }

        /// <summary>
        /// Retrieves an integer value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="value">When this method returns, contains the integer value associated with the specified key, if the key is found; otherwise, the default value for the type of the value parameter. This parameter is passed uninitialized.</param>
        /// <returns><c>true</c> if the configuration contains an element with the specified key; otherwise, <c>false</c>.</returns>
        public bool GetInt(string key, out int value)
        {
            return NativeMethods.YiniGetInt(m_handle, key, out value);
        }


        /// <summary>
        /// Retrieves an integer value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <returns>The integer value associated with the specified key, or <c>null</c> if the key is not found.</returns>
        public int? GetInt(string key)
        {
            if (NativeMethods.YiniGetInt(m_handle, key, out int value))
            {
                return value;
            }
            return null;
        }

        /// <summary>
        /// Retrieves a double-precision floating-point number for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="value">When this method returns, contains the double value associated with the specified key, if the key is found; otherwise, the default value for the type of the value parameter. This parameter is passed uninitialized.</param>
        /// <returns><c>true</c> if the configuration contains an element with the specified key; otherwise, <c>false</c>.</returns>
        public bool GetDouble(string key, out double value)
        {
            return NativeMethods.YiniGetDouble(m_handle, key, out value);
        }

        public double? GetDouble(string key)
        {
            if (NativeMethods.YiniGetDouble(m_handle, key, out double value))
            {
                return value;
            }
            return null;
        }

        /// <summary>
        /// Retrieves a boolean value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="value">When this method returns, contains the boolean value associated with the specified key, if the key is found; otherwise, the default value for the type of the value parameter. This parameter is passed uninitialized.</param>
        /// <returns><c>true</c> if the configuration contains an element with the specified key; otherwise, <c>false</c>.</returns>
        public bool GetBool(string key, out bool value)
        {
            return NativeMethods.YiniGetBool(m_handle, key, out value);
        }

        public bool? GetBool(string key)
        {
            if (NativeMethods.YiniGetBool(m_handle, key, out bool value))
            {
                return value;
            }
            return null;
        }

        /// <summary>
        /// Retrieves a string value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <returns>The string value associated with the specified key, or <c>null</c> if the key is not found.</returns>
        public string GetString(string key)
        {
            return NativeMethods.GetString(m_handle, key);
        }

        /// <summary>
        /// Retrieves an array of integers for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of integers, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public int[] GetIntArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new int[size];
            for (int i = 0; i < size; i++)
            {
                NativeMethods.YiniGetArrayItemAsInt(m_handle, key, i, out result[i]);
            }
            return result;
        }

        /// <summary>
        /// Retrieves an array of doubles for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of doubles, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public double[] GetDoubleArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new double[size];
            for (int i = 0; i < size; i++)
            {
                NativeMethods.YiniGetArrayItemAsDouble(m_handle, key, i, out result[i]);
            }
            return result;
        }

        /// <summary>
        /// Retrieves an array of booleans for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of booleans, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public bool[] GetBoolArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new bool[size];
            for (int i = 0; i < size; i++)
            {
                NativeMethods.YiniGetArrayItemAsBool(m_handle, key, i, out result[i]);
            }
            return result;
        }

        /// <summary>
        /// Retrieves an array of strings for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of strings, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public string[] GetStringArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new string[size];
            for (int i = 0; i < size; i++)
            {
                result[i] = NativeMethods.GetArrayItemAsString(m_handle, key, i);
            }
            return result;
        }

        /// <summary>
        /// Releases all resources used by the <see cref="YiniConfig"/> object.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases the unmanaged resources used by the <see cref="YiniConfig"/> and optionally releases the managed resources.
        /// </summary>
        /// <param name="disposing"><c>true</c> to release both managed and unmanaged resources; <c>false</c> to release only unmanaged resources.</param>
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

        /// <summary>
        /// Finalizer for the YiniConfig class.
        /// </summary>
        ~YiniConfig()
        {
            Dispose(false);
        }
    }
}
