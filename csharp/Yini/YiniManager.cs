using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;

namespace Yini
{
    /// <summary>
    /// Specifies the underlying data type of a <see cref="YiniValue"/>.
    /// This enum must be kept in sync with the C-API definition.
    /// </summary>
    public enum YiniValueType
    {
        /// <summary>Represents a null or uninitialized value.</summary>
        Null,
        /// <summary>Represents a boolean value.</summary>
        Bool,
        /// <summary>Represents a double-precision floating-point number.</summary>
        Double,
        /// <summary>Represents a string.</summary>
        String,
        /// <summary>Represents an array of <see cref="YiniValue"/> objects.</summary>
        Array,
        /// <summary>Represents a map with string keys and <see cref="YiniValue"/> values.</summary>
        Map,
        /// <summary>Represents a dynamic value that can be updated at runtime.</summary>
        Dyna
    }

    /// <summary>
    /// A managed wrapper for a native YINI value handle (`Yini_ValueHandle`).
    /// This class provides a type-safe way to interact with values retrieved from or created for the YINI system.
    /// It implements <see cref="IDisposable"/> to manage the lifetime of the underlying native handle.
    /// </summary>
    public class YiniValue : IDisposable
    {
        internal IntPtr Handle { get; private set; }
        private bool _disposed = false;

        internal YiniValue(IntPtr handle)
        {
            Handle = handle;
        }

        /// <summary>
        /// Gets the underlying data type of this <see cref="YiniValue"/>.
        /// </summary>
        public YiniValueType Type => YiniManager.YiniValue_GetType(Handle);

        // --- Factory methods for creating new values ---

        /// <summary>
        /// Creates a new <see cref="YiniValue"/> of type <see cref="YiniValueType.Double"/>.
        /// </summary>
        /// <param name="value">The double value.</param>
        /// <returns>A new <see cref="YiniValue"/> instance.</returns>
        public static YiniValue Create(double value) => new YiniValue(YiniManager.YiniValue_CreateDouble(value));

        /// <summary>
        /// Creates a new <see cref="YiniValue"/> of type <see cref="YiniValueType.String"/>.
        /// </summary>
        /// <param name="value">The string value.</param>
        /// <returns>A new <see cref="YiniValue"/> instance.</returns>
        public static YiniValue Create(string value) => new YiniValue(YiniManager.YiniValue_CreateString(value));

        /// <summary>
        /// Creates a new <see cref="YiniValue"/> of type <see cref="YiniValueType.Bool"/>.
        /// </summary>
        /// <param name="value">The boolean value.</param>
        /// <returns>A new <see cref="YiniValue"/> instance.</returns>
        public static YiniValue Create(bool value) => new YiniValue(YiniManager.YiniValue_CreateBool(value));

        /// <summary>
        /// Creates a new, empty <see cref="YiniValue"/> of type <see cref="YiniValueType.Array"/>.
        /// </summary>
        /// <returns>A new <see cref="YiniValue"/> instance representing an array.</returns>
        public static YiniValue CreateArray() => new YiniValue(YiniManager.YiniValue_CreateArray());

        /// <summary>
        /// Creates a new, empty <see cref="YiniValue"/> of type <see cref="YiniValueType.Map"/>.
        /// </summary>
        /// <returns>A new <see cref="YiniValue"/> instance representing a map.</returns>
        public static YiniValue CreateMap() => new YiniValue(YiniManager.YiniValue_CreateMap());

        // --- Methods to get data out of the value ---

        /// <summary>
        /// Retrieves the value as a <see cref="double"/>.
        /// </summary>
        /// <returns>The double value.</returns>
        /// <exception cref="InvalidCastException">Thrown if the value is not of type <see cref="YiniValueType.Double"/>.</exception>
        public double AsDouble()
        {
            if (YiniManager.YiniValue_GetDouble(Handle, out double value))
            {
                return value;
            }
            throw new InvalidCastException($"Cannot cast YiniValue of type {Type} to Double.");
        }

        /// <summary>
        /// Retrieves the value as a <see cref="string"/>.
        /// </summary>
        /// <returns>The string value.</returns>
        public string AsString()
        {
            int requiredSize = YiniManager.YiniValue_GetString(Handle, null, 0);
            if (requiredSize <= 0) return "";

            var buffer = new StringBuilder(requiredSize);
            YiniManager.YiniValue_GetString(Handle, buffer, buffer.Capacity);
            return buffer.ToString();
        }

        /// <summary>
        /// Retrieves the value as a <see cref="bool"/>.
        /// </summary>
        /// <returns>The boolean value.</returns>
        /// <exception cref="InvalidCastException">Thrown if the value is not of type <see cref="YiniValueType.Bool"/>.</exception>
        public bool AsBool()
        {
            if (YiniManager.YiniValue_GetBool(Handle, out bool value))
            {
                return value;
            }
            throw new InvalidCastException($"Cannot cast YiniValue of type {Type} to Bool.");
        }

        /// <summary>
        /// If this value is a dynamic value (<see cref="YiniValueType.Dyna"/>), this method retrieves its underlying, concrete value.
        /// </summary>
        /// <returns>A new <see cref="YiniValue"/> instance representing the concrete value.</returns>
        /// <exception cref="InvalidCastException">Thrown if the value is not of type <see cref="YiniValueType.Dyna"/>.</exception>
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

        /// <summary>
        /// Gets the number of elements in the array.
        /// Throws an exception if the value is not an array.
        /// </summary>
        public int ArraySize => YiniManager.YiniArray_GetSize(Handle);

        /// <summary>
        /// Gets the element at the specified index in the array.
        /// </summary>
        /// <param name="index">The zero-based index of the element to get.</param>
        /// <returns>A new <see cref="YiniValue"/> instance for the element at the specified index.</returns>
        /// <exception cref="IndexOutOfRangeException">Thrown if the index is out of range.</exception>
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

        /// <summary>
        /// Adds an element to the end of the array.
        /// </summary>
        /// <param name="element">The <see cref="YiniValue"/> to add. The value is copied, and the caller retains ownership of the passed instance.</param>
        public void AddArrayElement(YiniValue element)
        {
            // The C-API copies the value, we still own our 'element' handle.
            YiniManager.YiniArray_AddElement(Handle, element.Handle);
        }

        // --- Map methods ---

        /// <summary>
        /// Gets the number of key-value pairs in the map.
        /// Throws an exception if the value is not a map.
        /// </summary>
        public int MapSize => YiniManager.YiniMap_GetSize(Handle);

        /// <summary>
        /// Returns an enumerable collection of key-value pairs in the map.
        /// Each iteration yields a new <see cref="YiniValue"/> that must be disposed by the caller.
        /// </summary>
        /// <returns>An <see cref="IEnumerable{T}"/> of key-value pairs.</returns>
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

        /// <summary>
        /// Sets the value for a given key in the map. If the key already exists, its value is overwritten.
        /// </summary>
        /// <param name="key">The string key.</param>
        /// <param name="value">The <see cref="YiniValue"/> to set. The value is copied, and the caller retains ownership of the passed instance.</param>
        public void SetMapValue(string key, YiniValue value)
        {
            // The C-API copies the value, we still own our 'value' handle.
            YiniManager.YiniMap_SetValue(Handle, key, value.Handle);
        }

        #region IDisposable Implementation
        /// <summary>
        /// Releases the underlying native YINI value handle.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases the unmanaged resources used by the <see cref="YiniValue"/> and optionally releases the managed resources.
        /// </summary>
        /// <param name="disposing">true to release both managed and unmanaged resources; false to release only unmanaged resources.</param>
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

        /// <summary>
        /// Finalizer to ensure native resources are freed.
        /// </summary>
        ~YiniValue()
        {
            Dispose(false);
        }
        #endregion
    }

    /// <summary>
    /// Manages interactions with YINI files, including loading, saving, and accessing data.
    /// This class is the primary entry point for using the YINI library in .NET.
    /// It implements <see cref="IDisposable"/> to manage the lifetime of the underlying native manager instance.
    /// </summary>
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

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniManager"/> class.
        /// </summary>
        /// <exception cref="InvalidOperationException">Thrown if the native YINI manager cannot be created.</exception>
        public YiniManager()
        {
            _managerPtr = YiniManager_Create();
            if (_managerPtr == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to create YiniManager instance.");
            }
        }

        /// <summary>
        /// Loads and parses a YINI file from the specified path.
        /// </summary>
        /// <param name="filepath">The path to the YINI file.</param>
        /// <returns>true if the file was loaded successfully; otherwise, false.</returns>
        public bool Load(string filepath) => YiniManager_Load(_managerPtr, filepath);

        /// <summary>
        /// Saves any changes made to dynamic values back to the corresponding .ymeta file.
        /// </summary>
        public void SaveChanges() => YiniManager_SaveChanges(_managerPtr);

        /// <summary>
        /// Retrieves a value from the loaded YINI data.
        /// </summary>
        /// <param name="section">The name of the section.</param>
        /// <param name="key">The name of the key.</param>
        /// <returns>A <see cref="YiniValue"/> instance if the key is found; otherwise, null. The returned instance must be disposed by the caller.</returns>
        public YiniValue? GetValue(string section, string key)
        {
            // The C-API returns a new handle that we own.
            var valueHandle = YiniManager_GetValue(_managerPtr, section, key);
            return valueHandle == IntPtr.Zero ? null : new YiniValue(valueHandle);
        }

        /// <summary>
        /// Checks if a key exists in a given section.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <returns>true if the key exists; otherwise, false.</returns>
        public bool HasKey(string section, string key)
        {
            using (var value = GetValue(section, key))
            {
                return value != null;
            }
        }

        /// <summary>
        /// Sets a value for a specific key in a section.
        /// </summary>
        /// <param name="section">The name of the section.</param>
        /// <param name="key">The name of the key.</param>
        /// <param name="value">The <see cref="YiniValue"/> to set. The value is copied, so the caller retains ownership of the passed instance.</param>
        public void SetValue(string section, string key, YiniValue value)
        {
            // The C-API copies the value, we still own our 'value' handle.
            YiniManager_SetValue(_managerPtr, section, key, value.Handle);
        }

        // --- Convenience methods for primitive types ---

        /// <summary>
        /// Gets a double value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="defaultValue">The value to return if the key is not found or is not a double.</param>
        /// <returns>The double value or the default value.</returns>
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

        /// <summary>
        /// Gets a string value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="defaultValue">The value to return if the key is not found or is not a string.</param>
        /// <returns>The string value or the default value.</returns>
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

        /// <summary>
        /// Gets a boolean value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="defaultValue">The value to return if the key is not found or is not a boolean.</param>
        /// <returns>The boolean value or the default value.</returns>
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

        /// <summary>
        /// Sets a double value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="value">The double value to set.</param>
        public void SetDouble(string section, string key, double value)
        {
            using (var yiniValue = YiniValue.Create(value))
            {
                SetValue(section, key, yiniValue);
            }
        }

        /// <summary>
        /// Sets a string value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="value">The string value to set.</param>
        public void SetString(string section, string key, string value)
        {
            using (var yiniValue = YiniValue.Create(value))
            {
                SetValue(section, key, yiniValue);
            }
        }

        /// <summary>
        /// Sets a boolean value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="value">The boolean value to set.</param>
        public void SetBool(string section, string key, bool value)
        {
            using (var yiniValue = YiniValue.Create(value))
            {
                SetValue(section, key, yiniValue);
            }
        }

        /// <summary>
        /// Binds the data from a YINI section to a new instance of a specified type using reflection.
        /// Properties of the object are mapped to keys in the section.
        /// </summary>
        /// <typeparam name="T">The type of the object to bind to. Must have a parameterless constructor.</typeparam>
        /// <param name="section">The name of the section to bind from.</param>
        /// <returns>A new instance of type <typeparamref name="T"/> with its properties populated from the YINI data.</returns>
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

        /// <summary>
        /// Gets a list of a specified type from an array value in a YINI section.
        /// </summary>
        /// <typeparam name="T">The element type of the list.</typeparam>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <returns>A new <see cref="List{T}"/> containing the elements from the YINI array, or null if the key is not found or is not an array.</returns>
        public List<T>? GetList<T>(string section, string key)
        {
            using (var yiniValue = GetValue(section, key))
            {
                if (yiniValue == null || yiniValue.Type != YiniValueType.Array)
                {
                    return null;
                }

                var list = new List<T>();
                var elementType = typeof(T);

                for (int i = 0; i < yiniValue.ArraySize; i++)
                {
                    using (var element = yiniValue.GetArrayElement(i))
                    {
                        if (element != null)
                        {
                            var converted = ConvertYiniValue(element, elementType);
                            if (converted != null)
                            {
                                list.Add((T)converted);
                            }
                        }
                    }
                }
                return list;
            }
        }

        /// <summary>
        /// Gets a dictionary from a map value in a YINI section.
        /// </summary>
        /// <typeparam name="T">The value type of the dictionary.</typeparam>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <returns>A new <see cref="Dictionary{TKey, TValue}"/> with string keys, or null if the key is not found or is not a map.</returns>
        public Dictionary<string, T>? GetDictionary<T>(string section, string key)
        {
            using (var yiniValue = GetValue(section, key))
            {
                if (yiniValue == null || yiniValue.Type != YiniValueType.Map)
                {
                    return null;
                }

                var dict = new Dictionary<string, T>();
                var valueType = typeof(T);

                foreach (var kvp in yiniValue.AsMap())
                {
                    using (var element = kvp.Value)
                    {
                        if (element != null)
                        {
                            var converted = ConvertYiniValue(element, valueType);
                            if (converted != null)
                            {
                                dict[kvp.Key] = (T)converted;
                            }
                        }
                    }
                }
                return dict;
            }
        }

        #region IDisposable Implementation
        /// <summary>
        /// Releases the underlying native YINI manager instance.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases the unmanaged resources used by the <see cref="YiniManager"/> and optionally releases the managed resources.
        /// </summary>
        /// <param name="disposing">true to release both managed and unmanaged resources; false to release only unmanaged resources.</param>
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

        /// <summary>
        /// Finalizer to ensure native resources are freed.
        /// </summary>
        ~YiniManager()
        {
            Dispose(false);
        }
        #endregion
    }
}