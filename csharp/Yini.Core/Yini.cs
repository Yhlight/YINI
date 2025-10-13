using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace Yini.Core
{
    /// <summary>
    /// Represents the possible types of a YINI value.
    /// This must be kept in sync with the C++ counterpart.
    /// </summary>
    public enum ValueType
    {
        Null,
        Int,
        Double,
        Bool,
        String,
        Struct,
        Map,
        ArrayInt,
        ArrayDouble,
        ArrayBool,
        ArrayString
    }

    internal static class NativeMethods
    {
        private const string LibName = "YiniInterop";

        [DllImport(LibName, EntryPoint = "yini_create_from_file", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr YiniCreateFromFile(string filePath, out IntPtr outError);

        [DllImport(LibName, EntryPoint = "yini_create", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr YiniCreate();

        [DllImport(LibName, EntryPoint = "yini_set_int", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniSetInt(IntPtr handle, string key, int value);

        [DllImport(LibName, EntryPoint = "yini_set_double", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniSetDouble(IntPtr handle, string key, double value);

        [DllImport(LibName, EntryPoint = "yini_set_bool", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniSetBool(IntPtr handle, string key, bool value);

        [DllImport(LibName, EntryPoint = "yini_set_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniSetString(IntPtr handle, string key, string value);

        [DllImport(LibName, EntryPoint = "yini_save_to_file", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniSaveToFile(IntPtr handle, string filePath, out IntPtr outError);

        [DllImport(LibName, EntryPoint = "yini_free_error_string", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniFreeErrorString(IntPtr str);

        [DllImport(LibName, EntryPoint = "yini_destroy", CallingConvention = CallingConvention.Cdecl)]
        public static extern void YiniDestroy(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_type", CallingConvention = CallingConvention.Cdecl)]
        public static extern ValueType YiniGetType(IntPtr handle, string key);

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

        // --- Map Getters ---
        [DllImport(LibName, EntryPoint = "yini_get_map_size", CallingConvention = CallingConvention.Cdecl)]
        public static extern int YiniGetMapSize(IntPtr handle, string key);

        [DllImport(LibName, EntryPoint = "yini_get_map_key_at_index", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetMapKeyAtIndex(IntPtr handle, string key, int index);

        [DllImport(LibName, EntryPoint = "yini_get_map_value_type", CallingConvention = CallingConvention.Cdecl)]
        public static extern ValueType YiniGetMapValueType(IntPtr handle, string key, string subKey);

        [DllImport(LibName, EntryPoint = "yini_get_map_value_as_int", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetMapValueAsInt(IntPtr handle, string key, string subKey, out int outValue);

        [DllImport(LibName, EntryPoint = "yini_get_map_value_as_double", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetMapValueAsDouble(IntPtr handle, string key, string subKey, out double outValue);

        [DllImport(LibName, EntryPoint = "yini_get_map_value_as_bool", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetMapValueAsBool(IntPtr handle, string key, string subKey, out bool outValue);

        [DllImport(LibName, EntryPoint = "yini_get_map_value_as_string", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetMapValueAsString(IntPtr handle, string key, string subKey);

        // --- Struct Getters ---
        [DllImport(LibName, EntryPoint = "yini_get_struct_key", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetStructKey(IntPtr handle, string key);

        [DllImport(LibName, EntryPoint = "yini_get_struct_value_type", CallingConvention = CallingConvention.Cdecl)]
        public static extern ValueType YiniGetStructValueType(IntPtr handle, string key);

        [DllImport(LibName, EntryPoint = "yini_get_struct_value_as_int", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetStructValueAsInt(IntPtr handle, string key, out int outValue);

        [DllImport(LibName, EntryPoint = "yini_get_struct_value_as_double", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetStructValueAsDouble(IntPtr handle, string key, out double outValue);

        [DllImport(LibName, EntryPoint = "yini_get_struct_value_as_bool", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool YiniGetStructValueAsBool(IntPtr handle, string key, out bool outValue);

        [DllImport(LibName, EntryPoint = "yini_get_struct_value_as_string", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetStructValueAsString(IntPtr handle, string key);


        public static string? GetStringAndFree(IntPtr cstr)
        {
            if (cstr == IntPtr.Zero) return null;
            try
            {
                return Marshal.PtrToStringAnsi(cstr);
            }
            finally
            {
                YiniFreeString(cstr);
            }
        }

        public static string? GetMapKeyAtIndex(IntPtr handle, string key, int index)
        {
            IntPtr cstr = YiniGetMapKeyAtIndex(handle, key, index);
            return GetStringAndFree(cstr);
        }

        public static string? GetMapValueAsString(IntPtr handle, string key, string subKey)
        {
            IntPtr cstr = YiniGetMapValueAsString(handle, key, subKey);
            return GetStringAndFree(cstr);
        }

        public static string? GetStructKey(IntPtr handle, string key)
        {
            IntPtr cstr = YiniGetStructKey(handle, key);
            return GetStringAndFree(cstr);
        }

        public static string? GetStructValueAsString(IntPtr handle, string key)
        {
            IntPtr cstr = YiniGetStructValueAsString(handle, key);
            return GetStringAndFree(cstr);
        }

        public static string? GetString(IntPtr handle, string key)
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

        public static string? GetArrayItemAsString(IntPtr handle, string key, int index)
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
        /// Initializes a new, empty instance of the <see cref="YiniConfig"/> class in memory.
        /// </summary>
        public YiniConfig()
        {
            m_handle = NativeMethods.YiniCreate();
            if (m_handle == IntPtr.Zero)
            {
                throw new YiniException("Failed to create an empty YINI config handle.");
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniConfig"/> class by loading and parsing a YINI file.
        /// </summary>
        /// <param name="filePath">The path to the .yini file.</param>
        /// <exception cref="YiniException">Thrown if the native library fails to load or parse the file.</exception>
        public YiniConfig(string filePath)
        {
            m_handle = NativeMethods.YiniCreateFromFile(filePath, out IntPtr errorPtr);
            if (m_handle == IntPtr.Zero)
            {
                string errorMessage = "An unknown error occurred.";
                if (errorPtr != IntPtr.Zero)
                {
                    try
                    {
                        errorMessage = Marshal.PtrToStringAnsi(errorPtr) ?? "Failed to retrieve error message.";
                    }
                    finally
                    {
                        NativeMethods.YiniFreeErrorString(errorPtr);
                    }
                }
                throw new YiniException($"Failed to create YINI config: {errorMessage}");
            }
        }

        /// <summary>
        /// Retrieves an integer value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="value">When this method returns, contains the integer value associated with the specified key, if the key is found; otherwise, the default value for the type of the value parameter. This parameter is passed uninitialized.</param>
        /// <returns><c>true</c> if the configuration contains an element with the specified key; otherwise, <c>false</c>.</returns>
        [Obsolete("This method is obsolete. Use the nullable GetInt(string key) overload instead.", true)]
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
        [Obsolete("This method is obsolete. Use the nullable GetDouble(string key) overload instead.", true)]
        public bool GetDouble(string key, out double value)
        {
            return NativeMethods.YiniGetDouble(m_handle, key, out value);
        }

        /// <summary>
        /// Retrieves a double-precision floating-point number for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <returns>The double value associated with the specified key, or <c>null</c> if the key is not found.</returns>
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
        [Obsolete("This method is obsolete. Use the nullable GetBool(string key) overload instead.", true)]
        public bool GetBool(string key, out bool value)
        {
            return NativeMethods.YiniGetBool(m_handle, key, out value);
        }

        /// <summary>
        /// Retrieves a boolean value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <returns>The boolean value associated with the specified key, or <c>null</c> if the key is not found.</returns>
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
        public string? GetString(string key)
        {
            return NativeMethods.GetString(m_handle, key);
        }

        /// <summary>
        /// Retrieves an array of integers for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of integers, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public int?[]? GetIntArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new int?[size];
            for (int i = 0; i < size; i++)
            {
                if (NativeMethods.YiniGetArrayItemAsInt(m_handle, key, i, out int value))
                {
                    result[i] = value;
                }
                else
                {
                    result[i] = null;
                }
            }
            return result;
        }

        /// <summary>
        /// Retrieves an array of doubles for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of doubles, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public double?[]? GetDoubleArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new double?[size];
            for (int i = 0; i < size; i++)
            {
                if (NativeMethods.YiniGetArrayItemAsDouble(m_handle, key, i, out double value))
                {
                    result[i] = value;
                }
                else
                {
                    result[i] = null;
                }
            }
            return result;
        }

        /// <summary>
        /// Retrieves an array of booleans for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of booleans, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public bool?[]? GetBoolArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new bool?[size];
            for (int i = 0; i < size; i++)
            {
                if (NativeMethods.YiniGetArrayItemAsBool(m_handle, key, i, out bool value))
                {
                    result[i] = value;
                }
                else
                {
                    result[i] = null;
                }
            }
            return result;
        }

        /// <summary>
        /// Retrieves an array of strings for a specified key.
        /// </summary>
        /// <param name="key">The key of the array to retrieve.</param>
        /// <returns>An array of strings, or <c>null</c> if the key is not found or the value is not an array.</returns>
        public string?[]? GetStringArray(string key)
        {
            int size = NativeMethods.YiniGetArraySize(m_handle, key);
            if (size < 0) return null;
            var result = new string?[size];
            for (int i = 0; i < size; i++)
            {
                result[i] = NativeMethods.GetArrayItemAsString(m_handle, key, i);
            }
            return result;
        }

        /// <summary>
        /// Retrieves a value of a specified type for a given key.
        /// </summary>
        /// <typeparam name="T">The type of the value to retrieve. Supported types are int, double, bool, string, and their nullable and array counterparts.</typeparam>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <returns>The value associated with the specified key, cast to the specified type. Returns the default value for the type (e.g., 0 for int, null for string) if the key is not found.</returns>
        /// <exception cref="NotSupportedException">Thrown if the requested type <typeparamref name="T"/> is not supported.</exception>
        public T? Get<T>(string key)
        {
            object? value = null;
            Type targetType = typeof(T);

            if (targetType == typeof(int) || targetType == typeof(int?))
            {
                value = GetInt(key);
            }
            else if (targetType == typeof(double) || targetType == typeof(double?))
            {
                value = GetDouble(key);
            }
            else if (targetType == typeof(bool) || targetType == typeof(bool?))
            {
                value = GetBool(key);
            }
            else if (targetType == typeof(string))
            {
                value = GetString(key);
            }
            else if (targetType == typeof(int?[]))
            {
                value = GetIntArray(key);
            }
            else if (targetType == typeof(double?[]))
            {
                value = GetDoubleArray(key);
            }
            else if (targetType == typeof(bool?[]))
            {
                value = GetBoolArray(key);
            }
            else if (targetType == typeof(string?[]))
            {
                value = GetStringArray(key);
            }
            else
            {
                throw new NotSupportedException($"The type {targetType.Name} is not supported by YiniConfig.Get<T>.");
            }

            if (value == null)
            {
                return default; // Returns null for nullable types, 0 for int, etc.
            }

            return (T)value;
        }

        /// <summary>
        /// Retrieves an integer value for a specified key, or a default value if the key is not found.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="defaultValue">The default value to return if the key is not found.</param>
        /// <returns>The integer value associated with the specified key, or the default value.</returns>
        public int GetIntOrDefault(string key, int defaultValue)
        {
            return GetInt(key) ?? defaultValue;
        }

        /// <summary>
        /// Retrieves a double-precision floating-point number for a specified key, or a default value if the key is not found.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="defaultValue">The default value to return if the key is not found.</param>
        /// <returns>The double value associated with the specified key, or the default value.</returns>
        public double GetDoubleOrDefault(string key, double defaultValue)
        {
            return GetDouble(key) ?? defaultValue;
        }

        /// <summary>
        /// Retrieves a boolean value for a specified key, or a default value if the key is not found.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="defaultValue">The default value to return if the key is not found.</param>
        /// <returns>The boolean value associated with the specified key, or the default value.</returns>
        public bool GetBoolOrDefault(string key, bool defaultValue)
        {
            return GetBool(key) ?? defaultValue;
        }

        /// <summary>
        /// Retrieves a string value for a specified key, or a default value if the key is not found.
        /// </summary>
        /// <param name="key">The key of the value to retrieve (e.g., "Section.key").</param>
        /// <param name="defaultValue">The default value to return if the key is not found.</param>
        /// <returns>The string value associated with the specified key, or the default value.</returns>
        public string GetStringOrDefault(string key, string defaultValue)
        {
            return GetString(key) ?? defaultValue;
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

        /// <summary>
        /// Gets the value associated with the specified key as a raw object.
        /// </summary>
        /// <param name="key">The key of the value to get.</param>
        /// <returns>The value associated with the key, or null if the key is not found.</returns>
        public object? this[string key]
        {
            get
            {
                ValueType type = NativeMethods.YiniGetType(m_handle, key);
                switch (type)
                {
                    case ValueType.Int:
                        return GetInt(key);
                    case ValueType.Double:
                        return GetDouble(key);
                    case ValueType.Bool:
                        return GetBool(key);
                    case ValueType.String:
                        return GetString(key);
                    case ValueType.ArrayInt:
                        return GetIntArray(key);
                    case ValueType.ArrayDouble:
                        return GetDoubleArray(key);
                    case ValueType.ArrayBool:
                        return GetBoolArray(key);
                    case ValueType.ArrayString:
                        return GetStringArray(key);
                    case ValueType.Map:
                        return GetMap(key);
                    case ValueType.Struct:
                        return GetStruct(key);
                    case ValueType.Null:
                    default:
                        return null;
                }
            }
        }

        public Dictionary<string, object?>? GetMap(string key)
        {
            if (NativeMethods.YiniGetType(m_handle, key) != ValueType.Map)
            {
                return null;
            }

            int size = NativeMethods.YiniGetMapSize(m_handle, key);
            if (size < 0) return null;

            var result = new Dictionary<string, object?>(size);
            for (int i = 0; i < size; i++)
            {
                string? subKey = NativeMethods.GetMapKeyAtIndex(m_handle, key, i);
                if (subKey == null) continue;

                ValueType subType = NativeMethods.YiniGetMapValueType(m_handle, key, subKey);
                switch (subType)
                {
                    case ValueType.Int:
                        if (NativeMethods.YiniGetMapValueAsInt(m_handle, key, subKey, out int intVal)) result[subKey] = intVal;
                        break;
                    case ValueType.Double:
                        if (NativeMethods.YiniGetMapValueAsDouble(m_handle, key, subKey, out double dblVal)) result[subKey] = dblVal;
                        break;
                    case ValueType.Bool:
                        if (NativeMethods.YiniGetMapValueAsBool(m_handle, key, subKey, out bool blnVal)) result[subKey] = blnVal;
                        break;
                    case ValueType.String:
                        result[subKey] = NativeMethods.GetMapValueAsString(m_handle, key, subKey);
                        break;
                }
            }
            return result;
        }

        public KeyValuePair<string, object?>? GetStruct(string key)
        {
            if (NativeMethods.YiniGetType(m_handle, key) != ValueType.Struct)
            {
                return null;
            }

            string? structKey = NativeMethods.GetStructKey(m_handle, key);
            if (structKey == null) return null;

            ValueType valueType = NativeMethods.YiniGetStructValueType(m_handle, key);
            object? structValue = null;

            switch (valueType)
            {
                case ValueType.Int:
                    if (NativeMethods.YiniGetStructValueAsInt(m_handle, key, out int intVal)) structValue = intVal;
                    break;
                case ValueType.Double:
                    if (NativeMethods.YiniGetStructValueAsDouble(m_handle, key, out double dblVal)) structValue = dblVal;
                    break;
                case ValueType.Bool:
                    if (NativeMethods.YiniGetStructValueAsBool(m_handle, key, out bool blnVal)) structValue = blnVal;
                    break;
                case ValueType.String:
                    structValue = NativeMethods.GetStructValueAsString(m_handle, key);
                    break;
            }

            return new KeyValuePair<string, object?>(structKey, structValue);
        }

        /// <summary>
        /// Sets an integer value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to set (e.g., "Section.key").</param>
        /// <param name="value">The integer value to set.</param>
        public void SetValue(string key, int value)
        {
            NativeMethods.YiniSetInt(m_handle, key, value);
        }

        /// <summary>
        /// Sets a double value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to set (e.g., "Section.key").</param>
        /// <param name="value">The double value to set.</param>
        public void SetValue(string key, double value)
        {
            NativeMethods.YiniSetDouble(m_handle, key, value);
        }

        /// <summary>
        /// Sets a boolean value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to set (e.g., "Section.key").</param>
        /// <param name="value">The boolean value to set.</param>
        public void SetValue(string key, bool value)
        {
            NativeMethods.YiniSetBool(m_handle, key, value);
        }

        /// <summary>
        /// Sets a string value for a specified key.
        /// </summary>
        /// <param name="key">The key of the value to set (e.g., "Section.key").</param>
        /// <param name="value">The string value to set.</param>
        public void SetValue(string key, string value)
        {
            if (value == null)
            {
                // Or handle as an error, depending on desired behavior
                return;
            }
            NativeMethods.YiniSetString(m_handle, key, value);
        }

        /// <summary>
        /// Saves the current in-memory configuration to a .yini file.
        /// </summary>
        /// <param name="filePath">The path to the file where the configuration will be saved.</param>
        /// <exception cref="YiniException">Thrown if the native library fails to save the file.</exception>
        public void Save(string filePath)
        {
            if (!NativeMethods.YiniSaveToFile(m_handle, filePath, out IntPtr errorPtr))
            {
                string errorMessage = "An unknown error occurred during save.";
                if (errorPtr != IntPtr.Zero)
                {
                    try
                    {
                        errorMessage = Marshal.PtrToStringAnsi(errorPtr) ?? "Failed to retrieve error message.";
                    }
                    finally
                    {
                        NativeMethods.YiniFreeErrorString(errorPtr);
                    }
                }
                throw new YiniException($"Failed to save YINI config: {errorMessage}");
            }
        }
    }
}
