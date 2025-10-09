using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace Yini.Core
{
    public enum YiniValueType
    {
        Null,
        String,
        Int,
        Double,
        Bool,
        Array,
        Map
    }

    [StructLayout(LayoutKind.Explicit)]
    internal struct YiniValueUnion
    {
        [FieldOffset(0)]
        public IntPtr StringValue;

        [FieldOffset(0)]
        public int IntValue;

        [FieldOffset(0)]
        public double DoubleValue;

        [FieldOffset(0)]
        [MarshalAs(UnmanagedType.I1)]
        public bool BoolValue;

        [FieldOffset(0)]
        public IntPtr ArrayValue;

        [FieldOffset(0)]
        public IntPtr MapValue;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniValue
    {
        public YiniValueType Type;
        public YiniValueUnion As;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniArray
    {
        public ulong Size;
        public IntPtr Elements; // YiniValue**
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniMapEntry
    {
        public IntPtr Key; // const char*
        public IntPtr Value; // YiniValue*
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniMap
    {
        public ulong Size;
        public IntPtr Entries; // YiniMapEntry**
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniDynaValue
    {
        public IntPtr Value; // YiniValue*
        public IntPtr Backups; // YiniArray*
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniSetMapEntry
    {
        public IntPtr Key; // const char*
        public IntPtr Value; // YiniSetValue*
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniSetArray
    {
        public ulong Size;
        public IntPtr Elements; // YiniSetValue**
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniSetMap
    {
        public ulong Size;
        public IntPtr Entries; // YiniSetMapEntry**
    }

    [StructLayout(LayoutKind.Explicit)]
    internal struct YiniSetValueUnion
    {
        [FieldOffset(0)]
        public IntPtr StringValue;

        [FieldOffset(0)]
        public int IntValue;

        [FieldOffset(0)]
        public double DoubleValue;

        [FieldOffset(0)]
        [MarshalAs(UnmanagedType.I1)]
        public bool BoolValue;

        [FieldOffset(0)]
        public YiniSetArray ArrayValue;

        [FieldOffset(0)]
        public YiniSetMap MapValue;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct YiniSetValue
    {
        public YiniValueType Type;
        public YiniSetValueUnion As;
    }

    public class YiniConfig : IDisposable
    {
        private const string LibName = "YiniInterop";

        private IntPtr _handle;
        private bool _disposed = false;
        private bool _isDirty = false;
        private readonly string _filepath;

        internal void MarkAsDirty()
        {
            _isDirty = true;
        }

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

        [DllImport(LibName, EntryPoint = "yini_get_color", CallingConvention = CallingConvention.Cdecl)]
        private static extern Color YiniGetColor(IntPtr handle, string section, string key, Color defaultValue);

        [DllImport(LibName, EntryPoint = "yini_get_coord", CallingConvention = CallingConvention.Cdecl)]
        private static extern Coord YiniGetCoord(IntPtr handle, string section, string key, Coord defaultValue);

        [DllImport(LibName, EntryPoint = "yini_get_path", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetPathInternal(IntPtr handle, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_free_string", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniFreeString(IntPtr str);

        [DllImport(LibName, EntryPoint = "yini_free_config", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniFreeConfig(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_value", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetValue(IntPtr handle, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_free_value", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniFreeValue(IntPtr value);

        [DllImport(LibName, EntryPoint = "yini_set_string", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetString(IntPtr handle, string section, string key, string value);

        [DllImport(LibName, EntryPoint = "yini_set_int", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetInt(IntPtr handle, string section, string key, int value);

        [DllImport(LibName, EntryPoint = "yini_set_double", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetDouble(IntPtr handle, string section, string key, double value);

        [DllImport(LibName, EntryPoint = "yini_set_bool", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetBool(IntPtr handle, string section, string key, [MarshalAs(UnmanagedType.I1)] bool value);

        [DllImport(LibName, EntryPoint = "yini_set_color", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetColor(IntPtr handle, string section, string key, Color value);

        [DllImport(LibName, EntryPoint = "yini_set_coord", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetCoord(IntPtr handle, string section, string key, Coord value);

        [DllImport(LibName, EntryPoint = "yini_set_path", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetPath(IntPtr handle, string section, string key, string value);

        [DllImport(LibName, EntryPoint = "yini_set_value", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSetValue(IntPtr handle, string section, string key, IntPtr value);

        [DllImport(LibName, EntryPoint = "yini_save_file", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniSaveFile(IntPtr handle, string filepath);

        [DllImport(LibName, EntryPoint = "yini_get_dyna", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr YiniGetDyna(IntPtr handle, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_free_dyna", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YiniFreeDyna(IntPtr dyna);

        public YiniConfig(string filepath)
        {
            _filepath = filepath;
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

        public Color GetColor(string section, string key, Color defaultValue = default)
        {
            return YiniGetColor(_handle, section, key, defaultValue);
        }

        public Coord GetCoord(string section, string key, Coord defaultValue = default)
        {
            return YiniGetCoord(_handle, section, key, defaultValue);
        }

        public string GetPath(string section, string key, string defaultValue = null)
        {
            IntPtr cstr = YiniGetPathInternal(_handle, section, key);
            if (cstr == IntPtr.Zero)
            {
                return defaultValue;
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

        public object GetValue(string section, string key)
        {
            IntPtr valuePtr = YiniGetValue(_handle, section, key);
            if (valuePtr == IntPtr.Zero)
            {
                return null;
            }

            try
            {
                return MarshalYiniValue(valuePtr);
            }
            finally
            {
                YiniFreeValue(valuePtr);
            }
        }

        public void SetString(string section, string key, string value)
        {
            YiniSetString(_handle, section, key, value);
        }

        public void SetInt(string section, string key, int value)
        {
            YiniSetInt(_handle, section, key, value);
        }

        public void SetDouble(string section, string key, double value)
        {
            YiniSetDouble(_handle, section, key, value);
        }

        public void SetBool(string section, string key, bool value)
        {
            YiniSetBool(_handle, section, key, value);
        }

        public void SetColor(string section, string key, Color value)
        {
            YiniSetColor(_handle, section, key, value);
        }

        public void SetCoord(string section, string key, Coord value)
        {
            YiniSetCoord(_handle, section, key, value);
        }

        public void SetPath(string section, string key, string value)
        {
            YiniSetPath(_handle, section, key, value);
        }

        public void SetValue(string section, string key, object value)
        {
            IntPtr unmanagedValue = IntPtr.Zero;
            try
            {
                unmanagedValue = MarshalObjectToYiniSetValue(value);
                YiniSetValue(_handle, section, key, unmanagedValue);
            }
            finally
            {
                if (unmanagedValue != IntPtr.Zero)
                {
                    FreeUnmanagedYiniSetValue(unmanagedValue);
                }
            }
        }

        private IntPtr MarshalObjectToYiniSetValue(object obj)
        {
            var yiniSetValue = new YiniSetValue();
            IntPtr finalPtr = Marshal.AllocHGlobal(Marshal.SizeOf(yiniSetValue));

            if (obj == null)
            {
                yiniSetValue.Type = YiniValueType.Null;
                Marshal.StructureToPtr(yiniSetValue, finalPtr, false);
                return finalPtr;
            }

            var type = obj.GetType();

            if (type == typeof(string))
            {
                yiniSetValue.Type = YiniValueType.String;
                yiniSetValue.As.StringValue = Marshal.StringToHGlobalAnsi((string)obj);
            }
            else if (type == typeof(int))
            {
                yiniSetValue.Type = YiniValueType.Int;
                yiniSetValue.As.IntValue = (int)obj;
            }
            else if (type == typeof(double) || type == typeof(float))
            {
                yiniSetValue.Type = YiniValueType.Double;
                yiniSetValue.As.DoubleValue = Convert.ToDouble(obj);
            }
            else if (type == typeof(bool))
            {
                yiniSetValue.Type = YiniValueType.Bool;
                yiniSetValue.As.BoolValue = (bool)obj;
            }
            else if (obj is object[] objArray) // Array
            {
                yiniSetValue.Type = YiniValueType.Array;
                var arrayStruct = new YiniSetArray
                {
                    Size = (ulong)objArray.Length,
                    Elements = Marshal.AllocHGlobal(IntPtr.Size * objArray.Length)
                };

                for (int i = 0; i < objArray.Length; i++)
                {
                    IntPtr elementPtr = MarshalObjectToYiniSetValue(objArray[i]);
                    Marshal.WriteIntPtr(arrayStruct.Elements, i * IntPtr.Size, elementPtr);
                }
                yiniSetValue.As.ArrayValue = arrayStruct;
            }
            else if (obj is Dictionary<string, object> objDict) // Map
            {
                yiniSetValue.Type = YiniValueType.Map;
                var mapStruct = new YiniSetMap
                {
                    Size = (ulong)objDict.Count,
                    Entries = Marshal.AllocHGlobal(IntPtr.Size * objDict.Count)
                };

                int i = 0;
                foreach (var kvp in objDict)
                {
                    var entryStruct = new YiniSetMapEntry
                    {
                        Key = Marshal.StringToHGlobalAnsi(kvp.Key),
                        Value = MarshalObjectToYiniSetValue(kvp.Value)
                    };
                    IntPtr entryPtr = Marshal.AllocHGlobal(Marshal.SizeOf(entryStruct));
                    Marshal.StructureToPtr(entryStruct, entryPtr, false);

                    Marshal.WriteIntPtr(mapStruct.Entries, i * IntPtr.Size, entryPtr);
                    i++;
                }
                yiniSetValue.As.MapValue = mapStruct;
            }
            else
            {
                Marshal.FreeHGlobal(finalPtr);
                throw new NotSupportedException($"Type {type.Name} is not supported for setting in YINI.");
            }

            Marshal.StructureToPtr(yiniSetValue, finalPtr, false);
            return finalPtr;
        }

        private void FreeUnmanagedYiniSetValue(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero) return;

            var yiniSetValue = Marshal.PtrToStructure<YiniSetValue>(ptr);

            switch (yiniSetValue.Type)
            {
                case YiniValueType.String:
                    Marshal.FreeHGlobal(yiniSetValue.As.StringValue);
                    break;
                case YiniValueType.Array:
                    var arrayStruct = yiniSetValue.As.ArrayValue;
                    for (int i = 0; i < (int)arrayStruct.Size; i++)
                    {
                        IntPtr elementPtr = Marshal.ReadIntPtr(arrayStruct.Elements, i * IntPtr.Size);
                        FreeUnmanagedYiniSetValue(elementPtr);
                    }
                    Marshal.FreeHGlobal(arrayStruct.Elements);
                    break;
                case YiniValueType.Map:
                    var mapStruct = yiniSetValue.As.MapValue;
                    for (int i = 0; i < (int)mapStruct.Size; i++)
                    {
                        IntPtr entryPtr = Marshal.ReadIntPtr(mapStruct.Entries, i * IntPtr.Size);
                        var entryStruct = Marshal.PtrToStructure<YiniSetMapEntry>(entryPtr);
                        Marshal.FreeHGlobal(entryStruct.Key);
                        FreeUnmanagedYiniSetValue(entryStruct.Value);
                        Marshal.FreeHGlobal(entryPtr);
                    }
                    Marshal.FreeHGlobal(mapStruct.Entries);
                    break;
            }

            Marshal.FreeHGlobal(ptr);
        }

        public void Save(string filepath)
        {
            YiniSaveFile(_handle, filepath);
        }

        public void Save()
        {
            Save(_filepath);
        }

        private object MarshalYiniValue(IntPtr valuePtr)
        {
            if (valuePtr == IntPtr.Zero) return null;

            YiniValue value = Marshal.PtrToStructure<YiniValue>(valuePtr);

            switch (value.Type)
            {
                case YiniValueType.Null:
                    return null;
                case YiniValueType.String:
                    return Marshal.PtrToStringAnsi(value.As.StringValue);
                case YiniValueType.Int:
                    return value.As.IntValue;
                case YiniValueType.Double:
                    return value.As.DoubleValue;
                case YiniValueType.Bool:
                    return value.As.BoolValue;
                case YiniValueType.Array:
                    return MarshalYiniArray(value.As.ArrayValue);
                case YiniValueType.Map:
                    return MarshalYiniMap(value.As.MapValue);
                default:
                    throw new NotSupportedException($"Unsupported YiniValueType: {value.Type}");
            }
        }

        private object[] MarshalYiniArray(IntPtr arrayPtr)
        {
            if (arrayPtr == IntPtr.Zero) return null;

            YiniArray array = Marshal.PtrToStructure<YiniArray>(arrayPtr);
            var result = new object[array.Size];

            for (ulong i = 0; i < array.Size; i++)
            {
                IntPtr elementPtr = Marshal.ReadIntPtr(array.Elements, (int)i * IntPtr.Size);
                result[i] = MarshalYiniValue(elementPtr);
            }
            return result;
        }

        private Dictionary<string, object> MarshalYiniMap(IntPtr mapPtr)
        {
            if (mapPtr == IntPtr.Zero) return null;

            YiniMap map = Marshal.PtrToStructure<YiniMap>(mapPtr);
            var result = new Dictionary<string, object>((int)map.Size);

            for (ulong i = 0; i < map.Size; i++)
            {
                IntPtr entryPtr = Marshal.ReadIntPtr(map.Entries, (int)i * IntPtr.Size);
                YiniMapEntry entry = Marshal.PtrToStructure<YiniMapEntry>(entryPtr);

                string key = Marshal.PtrToStringAnsi(entry.Key);
                object value = MarshalYiniValue(entry.Value);
                result[key] = value;
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
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                {
                    if (_isDirty)
                    {
                        Save();
                    }
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

        public DynaValue<T> GetDyna<T>(string section, string key)
        {
            IntPtr dynaPtr = YiniGetDyna(_handle, section, key);
            if (dynaPtr == IntPtr.Zero)
            {
                return null;
            }

            try
            {
                YiniDynaValue cDyna = Marshal.PtrToStructure<YiniDynaValue>(dynaPtr);
                T currentValue = (T)MarshalYiniValue(cDyna.Value);

                var backups = MarshalYiniArray(cDyna.Backups)?.Cast<T>() ?? Enumerable.Empty<T>();

                return new DynaValue<T>(this, section, key, currentValue, backups);
            }
            finally
            {
                YiniFreeDyna(dynaPtr);
            }
        }
    }
}