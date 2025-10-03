using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;

namespace Yini
{
    // Enum to represent the type of a YiniValue, must match the C-API
    public enum YiniValueType
    {
        Null,
        Bool,
        Double,
        String,
        Array,
        Map,
        Dyna
    }

    // A wrapper for the native Yini_ValueHandle
    public class YiniValue : IDisposable
    {
        internal IntPtr Handle { get; private set; }
        private bool _disposed = false;

        internal YiniValue(IntPtr handle)
        {
            Handle = handle;
        }

        public YiniValueType Type => YiniManager.YiniValue_GetType(Handle);

        // --- Factory methods for creating new values ---
        public static YiniValue Create(double value) => new YiniValue(YiniManager.YiniValue_CreateDouble(value));
        public static YiniValue Create(string value) => new YiniValue(YiniManager.YiniValue_CreateString(value));
        public static YiniValue Create(bool value) => new YiniValue(YiniManager.YiniValue_CreateBool(value));
        public static YiniValue CreateArray() => new YiniValue(YiniManager.YiniValue_CreateArray());
        public static YiniValue CreateMap() => new YiniValue(YiniManager.YiniValue_CreateMap());

        // --- Methods to get data out of the value ---
        public double AsDouble()
        {
            if (YiniManager.YiniValue_GetDouble(Handle, out double value))
            {
                return value;
            }
            throw new InvalidCastException($"Cannot cast YiniValue of type {Type} to Double.");
        }

        public string AsString()
        {
            int requiredSize = YiniManager.YiniValue_GetString(Handle, null, 0);
            if (requiredSize <= 0) return "";

            var buffer = new StringBuilder(requiredSize);
            YiniManager.YiniValue_GetString(Handle, buffer, buffer.Capacity);
            return buffer.ToString();
        }

        public bool AsBool()
        {
            if (YiniManager.YiniValue_GetBool(Handle, out bool value))
            {
                return value;
            }
            throw new InvalidCastException($"Cannot cast YiniValue of type {Type} to Bool.");
        }

        public YiniValue AsDynaValue()
        {
            var dynaHandle = YiniManager.YiniValue_GetDynaValue(Handle);
            if (dynaHandle == IntPtr.Zero)
            {
                throw new InvalidCastException("Value is not a dynamic value.");
            }
            // The C-API returns a new handle that we own.
            return new YiniValue(dynaHandle);
        }

        // --- Array methods ---
        public int ArraySize => YiniManager.YiniArray_GetSize(Handle);

        public YiniValue GetArrayElement(int index)
        {
            var elementHandle = YiniManager.YiniArray_GetElement(Handle, index);
            if (elementHandle == IntPtr.Zero)
            {
                throw new IndexOutOfRangeException();
            }
            // The C-API returns a new handle that we own.
            return new YiniValue(elementHandle);
        }

        public void AddArrayElement(YiniValue element)
        {
            // The C-API copies the value, we still own our 'element' handle.
            YiniManager.YiniArray_AddElement(Handle, element.Handle);
        }

        // --- Map methods ---
        public int MapSize => YiniManager.YiniMap_GetSize(Handle);

        public IEnumerable<KeyValuePair<string, YiniValue>> AsMap()
        {
            int size = MapSize;
            if (size < 0) yield break;

            for (int i = 0; i < size; i++)
            {
                // Get key using the safe two-call pattern
                int keySize = YiniManager.YiniMap_GetKeyAt(Handle, i, null, 0);
                if (keySize <= 0) continue;

                var keyBuffer = new StringBuilder(keySize);
                YiniManager.YiniMap_GetKeyAt(Handle, i, keyBuffer, keySize);
                string key = keyBuffer.ToString();

                // Get value handle. The C-API returns a new handle that we own.
                IntPtr valueHandle = YiniManager.YiniMap_GetValueAt(Handle, i);
                if (valueHandle == IntPtr.Zero) continue;

                yield return new KeyValuePair<string, YiniValue>(key, new YiniValue(valueHandle));
            }
        }

        public void SetMapValue(string key, YiniValue value)
        {
            // The C-API copies the value, we still own our 'value' handle.
            YiniManager.YiniMap_SetValue(Handle, key, value.Handle);
        }

        #region IDisposable Implementation
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (Handle != IntPtr.Zero)
                {
                    YiniManager.YiniValue_Destroy(Handle);
                    Handle = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~YiniValue()
        {
            Dispose(false);
        }
        #endregion
    }


    public class YiniManager : IDisposable
    {
        private const string LibName = "Yini"; // Assumes libYini.so or Yini.dll
        private IntPtr _managerPtr;
        private bool _disposed = false;

        #region PInvoke Signatures
        // Manager
        [DllImport(LibName, EntryPoint = "yini_manager_create")]
        internal static extern IntPtr YiniManager_Create();

        [DllImport(LibName, EntryPoint = "yini_manager_destroy")]
        internal static extern void YiniManager_Destroy(IntPtr manager);

        [DllImport(LibName, EntryPoint = "yini_manager_load")]
        internal static extern bool YiniManager_Load(IntPtr manager, string filepath);

        [DllImport(LibName, EntryPoint = "yini_manager_save_changes")]
        internal static extern void YiniManager_SaveChanges(IntPtr manager);

        [DllImport(LibName, EntryPoint = "yini_manager_get_value")]
        internal static extern IntPtr YiniManager_GetValue(IntPtr manager, string section, string key);

        [DllImport(LibName, EntryPoint = "yini_manager_set_value")]
        internal static extern void YiniManager_SetValue(IntPtr manager, string section, string key, IntPtr valueHandle);

        // Value Handles
        [DllImport(LibName, EntryPoint = "yini_value_destroy")]
        internal static extern void YiniValue_Destroy(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_value_get_type")]
        internal static extern YiniValueType YiniValue_GetType(IntPtr handle);

        // Create Value Handles
        [DllImport(LibName, EntryPoint = "yini_value_create_double")]
        internal static extern IntPtr YiniValue_CreateDouble(double value);

        [DllImport(LibName, EntryPoint = "yini_value_create_string")]
        internal static extern IntPtr YiniValue_CreateString(string value);

        [DllImport(LibName, EntryPoint = "yini_value_create_bool")]
        internal static extern IntPtr YiniValue_CreateBool(bool value);

        [DllImport(LibName, EntryPoint = "yini_value_create_array")]
        internal static extern IntPtr YiniValue_CreateArray();

        [DllImport(LibName, EntryPoint = "yini_value_create_map")]
        internal static extern IntPtr YiniValue_CreateMap();

        // Get Data from Value Handles
        [DllImport(LibName, EntryPoint = "yini_value_get_double")]
        internal static extern bool YiniValue_GetDouble(IntPtr handle, out double outValue);

        [DllImport(LibName, EntryPoint = "yini_value_get_string")]
        internal static extern int YiniValue_GetString(IntPtr handle, StringBuilder? outBuffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_value_get_bool")]
        internal static extern bool YiniValue_GetBool(IntPtr handle, out bool outValue);

        [DllImport(LibName, EntryPoint = "yini_value_get_dyna_value")]
        internal static extern IntPtr YiniValue_GetDynaValue(IntPtr handle);

        // Array Manipulation
        [DllImport(LibName, EntryPoint = "yini_array_get_size")]
        internal static extern int YiniArray_GetSize(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_array_get_element")]
        internal static extern IntPtr YiniArray_GetElement(IntPtr handle, int index);

        [DllImport(LibName, EntryPoint = "yini_array_add_element")]
        internal static extern void YiniArray_AddElement(IntPtr arrayHandle, IntPtr elementHandle);

        // Map Manipulation
        [DllImport(LibName, EntryPoint = "yini_map_get_size")]
        internal static extern int YiniMap_GetSize(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_map_get_value_at")]
        internal static extern IntPtr YiniMap_GetValueAt(IntPtr handle, int index);

        [DllImport(LibName, EntryPoint = "yini_map_get_key_at")]
        internal static extern int YiniMap_GetKeyAt(IntPtr handle, int index, StringBuilder? outBuffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_map_set_value")]
        internal static extern void YiniMap_SetValue(IntPtr mapHandle, string key, IntPtr valueHandle);
        #endregion

        public YiniManager()
        {
            _managerPtr = YiniManager_Create();
            if (_managerPtr == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to create YiniManager instance.");
            }
        }

        public bool Load(string filepath) => YiniManager_Load(_managerPtr, filepath);
        public void SaveChanges() => YiniManager_SaveChanges(_managerPtr);

        public YiniValue? GetValue(string section, string key)
        {
            // The C-API returns a new handle that we own.
            var valueHandle = YiniManager_GetValue(_managerPtr, section, key);
            return valueHandle == IntPtr.Zero ? null : new YiniValue(valueHandle);
        }

        public void SetValue(string section, string key, YiniValue value)
        {
            // The C-API copies the value, we still own our 'value' handle.
            YiniManager_SetValue(_managerPtr, section, key, value.Handle);
        }

        // --- Convenience methods for primitive types ---
        public double GetDouble(string section, string key, double defaultValue = 0.0)
        {
            using (var value = GetValue(section, key))
            {
                if (value != null && value.Type == YiniValueType.Double)
                {
                    return value.AsDouble();
                }
            }
            return defaultValue;
        }

        public string GetString(string section, string key, string defaultValue = "")
        {
            using (var value = GetValue(section, key))
            {
                if (value != null && value.Type == YiniValueType.String)
                {
                    return value.AsString();
                }
            }
            return defaultValue;
        }

        public bool GetBool(string section, string key, bool defaultValue = false)
        {
            using (var value = GetValue(section, key))
            {
                if (value != null && value.Type == YiniValueType.Bool)
                {
                    return value.AsBool();
                }
            }
            return defaultValue;
        }

        public void SetDouble(string section, string key, double value)
        {
            using (var yiniValue = YiniValue.Create(value))
            {
                SetValue(section, key, yiniValue);
            }
        }

        public void SetString(string section, string key, string value)
        {
            using (var yiniValue = YiniValue.Create(value))
            {
                SetValue(section, key, yiniValue);
            }
        }

        public void SetBool(string section, string key, bool value)
        {
            using (var yiniValue = YiniValue.Create(value))
            {
                SetValue(section, key, yiniValue);
            }
        }

        public T Bind<T>(string section) where T : new()
        {
            var instance = new T();
            var properties = typeof(T).GetProperties(BindingFlags.Public | BindingFlags.Instance);

            foreach (var prop in properties)
            {
                if (!prop.CanWrite) continue;

                var keyAttribute = prop.GetCustomAttribute<YiniKeyAttribute>();
                string key = keyAttribute?.Key ?? prop.Name.ToLower();
                Type propType = prop.PropertyType;

                using (var yiniValue = GetValue(section, key))
                {
                    if (yiniValue == null) continue;

                    object? propValue = ConvertYiniValue(yiniValue, propType);
                    if (propValue != null)
                    {
                        prop.SetValue(instance, propValue);
                    }
                }
            }

            return instance;
        }

        private object? ConvertYiniValue(YiniValue yiniValue, Type targetType)
        {
            switch (yiniValue.Type)
            {
                case YiniValueType.Double:
                    return Convert.ChangeType(yiniValue.AsDouble(), targetType);
                case YiniValueType.String:
                    return yiniValue.AsString();
                case YiniValueType.Bool:
                    return yiniValue.AsBool();
                case YiniValueType.Array:
                    return ConvertYiniArray(yiniValue, targetType);
                case YiniValueType.Map:
                    return ConvertYiniMap(yiniValue, targetType);
                case YiniValueType.Dyna:
                    using(var innerValue = yiniValue.AsDynaValue())
                    {
                        return ConvertYiniValue(innerValue, targetType);
                    }
                default:
                    return null;
            }
        }

        private object? ConvertYiniArray(YiniValue yiniValue, Type targetType)
        {
            if (!targetType.IsGenericType || (targetType.GetGenericTypeDefinition() != typeof(List<>) && targetType.GetGenericTypeDefinition() != typeof(IList<>)))
            {
                return null;
            }

            var elementType = targetType.GetGenericArguments()[0];
            var list = (IList)Activator.CreateInstance(typeof(List<>).MakeGenericType(elementType))!;

            int size = yiniValue.ArraySize;
            for (int i = 0; i < size; i++)
            {
                using (var element = yiniValue.GetArrayElement(i))
                {
                    list.Add(ConvertYiniValue(element, elementType));
                }
            }
            return list;
        }

        private object? ConvertYiniMap(YiniValue yiniValue, Type targetType)
        {
            if (!targetType.IsGenericType || (targetType.GetGenericTypeDefinition() != typeof(Dictionary<,>) && targetType.GetGenericTypeDefinition() != typeof(IDictionary<,>)))
            {
                 return null;
            }

            var keyType = targetType.GetGenericArguments()[0];
            var valueType = targetType.GetGenericArguments()[1];

            if (keyType != typeof(string)) return null; // Only support string keys for now

            var dict = (IDictionary)Activator.CreateInstance(typeof(Dictionary<,>).MakeGenericType(keyType, valueType))!;

            foreach (var kvp in yiniValue.AsMap())
            {
                using (var element = kvp.Value)
                {
                    dict[kvp.Key] = ConvertYiniValue(element, valueType);
                }
            }
            return dict;
        }

        #region IDisposable Implementation
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_managerPtr != IntPtr.Zero)
                {
                    YiniManager_Destroy(_managerPtr);
                    _managerPtr = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~YiniManager()
        {
            Dispose(false);
        }
        #endregion
    }
}