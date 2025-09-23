using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;

namespace Yini
{
    // C# representation of the C enum
    public enum YiniValueType
    {
        Uninitialized, String, Int64, Double, Bool, Array, Path, Coord, Color, Object
    }

    /// <summary>
    /// A high-level, safe wrapper for a loaded YINI file handle.
    /// Implements IDisposable for use in 'using' blocks to ensure native memory is freed.
    /// </summary>
    public class YiniHandle : IDisposable
    {
        private IntPtr handle;
        private bool isDisposed = false;

        public YiniHandle(string filepath)
        {
            handle = Native.Load(filepath);
            if (handle == IntPtr.Zero)
            {
                throw new InvalidOperationException($"Failed to load YINI file at '{filepath}'. Check console for C++ errors.");
            }
        }

        public YiniValue GetValue(string section, string key)
        {
            IntPtr valueHandle = Native.GetValue(handle, section, key);
            return valueHandle == IntPtr.Zero ? null : new YiniValue(valueHandle);
        }

        public T GetValueAs<T>(string section, string key)
        {
            StringBuilder buffer = new StringBuilder(4096);
            int bytesWritten = Native.GetValueAsJson(handle, section, key, buffer, buffer.Capacity);
            if (bytesWritten == -1) return default(T);
            return JsonSerializer.Deserialize<T>(buffer.ToString());
        }

        public void Dispose() { Dispose(true); GC.SuppressFinalize(this); }
        protected virtual void Dispose(bool disposing)
        {
            if (!isDisposed)
            {
                if (handle != IntPtr.Zero)
                {
                    Native.Free(handle);
                    handle = IntPtr.Zero;
                }
                isDisposed = true;
            }
        }
        ~YiniHandle() { Dispose(false); }
    }

    /// <summary>
    /// A wrapper for a handle to a single YINI value.
    /// Provides methods to get the type and concrete value.
    /// </summary>
    public class YiniValue
    {
        internal readonly IntPtr handle;
        public YiniValue(IntPtr handle) { this.handle = handle; }

        public YiniValueType Type => Native.GetValueType(handle);

        public string AsString()
        {
            StringBuilder buffer = new StringBuilder(1024);
            if (Native.ValueAsString(handle, buffer, buffer.Capacity) != -1) return buffer.ToString();
            return null;
        }

        public string AsJson()
        {
            StringBuilder buffer = new StringBuilder(4096);
            if (Native.ValueAsJson(handle, buffer, buffer.Capacity) != -1) return buffer.ToString();
            return null;
        }

        public T As<T>()
        {
            string json = AsJson();
            if (json == null) return default(T);
            return JsonSerializer.Deserialize<T>(json);
        }

        public long? AsInt64()
        {
            if (Native.ValueAsInt64(handle, out long value) == 1) return value;
            return null;
        }

        public YiniArray AsArray()
        {
            IntPtr arrayHandle = Native.ValueAsArray(handle);
            return arrayHandle == IntPtr.Zero ? null : new YiniArray(arrayHandle);
        }

        public string AsPath()
        {
            StringBuilder buffer = new StringBuilder(1024);
            if (Native.ValueAsPath(handle, buffer, buffer.Capacity) != -1) return buffer.ToString();
            return null;
        }
    }

    /// <summary>
    /// A high-level wrapper for a YINI array handle, allowing iteration.
    /// </summary>
    public class YiniArray : IEnumerable<YiniValue>
    {
        private readonly IntPtr handle;
        public YiniArray(IntPtr handle) { this.handle = handle; }

        public int Count => Native.ArrayGetSize(handle);

        public YiniValue this[int index]
        {
            get
            {
                if (index < 0 || index >= Count) throw new IndexOutOfRangeException();
                IntPtr valueHandle = Native.ArrayGetValue(handle, index);
                if (valueHandle == IntPtr.Zero) throw new InvalidOperationException("Failed to get value from array at index " + index);
                return new YiniValue(valueHandle);
            }
        }

        public IEnumerator<YiniValue> GetEnumerator()
        {
            int count = Count;
            for (int i = 0; i < count; i++)
            {
                yield return this[i];
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }


    /// <summary>
    /// Contains the low-level P/Invoke DllImport definitions.
    /// </summary>
    internal static class Native
    {
        private const string LibName = "yini";

        // --- Main Handle ---
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_load")]
        public static extern IntPtr Load(string filepath);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_free")]
        public static extern void Free(IntPtr handle);

        // --- JSON API ---
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_value_as_json")]
        public static extern int GetValueAsJson(IntPtr handle, string section, string key, StringBuilder outBuffer, int bufferSize);

        // --- Granular API ---
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_get_value")]
        public static extern IntPtr GetValue(IntPtr handle, string section, string key);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_value_get_type")]
        public static extern YiniValueType GetValueType(IntPtr valueHandle);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_value_as_string")]
        public static extern int ValueAsString(IntPtr valueHandle, StringBuilder outBuffer, int bufferSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_value_as_path")]
        public static extern int ValueAsPath(IntPtr valueHandle, StringBuilder outBuffer, int bufferSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_value_as_int64")]
        public static extern int ValueAsInt64(IntPtr valueHandle, out long outValue);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_value_as_array")]
        public static extern IntPtr ValueAsArray(IntPtr valueHandle);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_array_get_size")]
        public static extern int ArrayGetSize(IntPtr arrayHandle);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_array_get_value")]
        public static extern IntPtr ArrayGetValue(IntPtr arrayHandle, int index);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "yini_value_as_json")]
        public static extern int ValueAsJson(IntPtr valueHandle, StringBuilder outBuffer, int bufferSize);
    }
}
