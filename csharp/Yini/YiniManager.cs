using System;
using System.Buffers;
using System.Runtime.InteropServices;
using System.Text;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;
using System.Numerics;
using System.Linq;

namespace Yini
{
    /// <summary>
    /// Holds information about a key defined in a YINI schema.
    /// </summary>
    public class SchemaKeyInfo
    {
        /// <summary> The name of the key. </summary>
        public string Name { get; internal set; } = "";
        /// <summary> The expected type of the key (e.g., "string", "integer"). </summary>
        public string TypeName { get; internal set; } = "";
        /// <summary> Whether the key is required to be present. </summary>
        public bool IsRequired { get; internal set; }
    }

    /// <summary>
    /// Represents an error that occurs during YINI operations.
    /// </summary>
    public class YiniException : Exception
    {
        /// <summary> The line number where the error occurred. </summary>
        public int Line { get; }
        /// <summary> The column number where the error occurred. </summary>
        public int Column { get; }
        /// <summary>
        /// Initializes a new instance of the <see cref="YiniException"/> class.
        /// </summary>
        public YiniException() { }

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniException"/> class with a specified error message.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        public YiniException(string message) : base(message) { }

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniException"/> class with a specified error message and location.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="line">The line number where the error occurred.</param>
        /// <param name="column">The column number where the error occurred.</param>
        public YiniException(string message, int line, int column) : base(message)
        {
            Line = line;
            Column = column;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniException"/> class with a specified error message and a reference to the inner exception that is the cause of this exception.
        /// </summary>
        /// <param name="message">The error message that explains the reason for the exception.</param>
        /// <param name="inner">The exception that is the cause of the current exception, or a null reference if no inner exception is specified.</param>
        public YiniException(string message, Exception inner) : base(message, inner) { }
    }

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
        public unsafe string AsString()
        {
            int requiredSize = YiniManager.YiniValue_GetString(Handle, null, 0);
            if (requiredSize <= 0) return "";

            byte[]? rentedBuffer = null;
            try
            {
                rentedBuffer = ArrayPool<byte>.Shared.Rent(requiredSize);
                fixed (byte* buffer = rentedBuffer)
                {
                    YiniManager.YiniValue_GetString(Handle, buffer, requiredSize);
                    return Encoding.UTF8.GetString(buffer, requiredSize - 1); // Exclude null terminator
                }
            }
            finally
            {
                if (rentedBuffer != null)
                {
                    ArrayPool<byte>.Shared.Return(rentedBuffer);
                }
            }
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
        /// Returns a list of key-value pairs in the map.
        /// </summary>
        /// <returns>A <see cref="List{T}"/> of key-value pairs.</returns>
        public unsafe List<KeyValuePair<string, YiniValue>> AsMap()
        {
            var list = new List<KeyValuePair<string, YiniValue>>();
            int size = MapSize;
            if (size < 0) return list;

            for (int i = 0; i < size; i++)
            {
                // Get key using the safe two-call pattern
                int keySize = YiniManager.YiniMap_GetKeyAt(Handle, i, null, 0);
                if (keySize <= 0) continue;

                byte[]? rentedBuffer = null;
                try
                {
                    rentedBuffer = ArrayPool<byte>.Shared.Rent(keySize);
                    string key;
                    fixed (byte* keyBuffer = rentedBuffer)
                    {
                        YiniManager.YiniMap_GetKeyAt(Handle, i, keyBuffer, keySize);
                        key = Encoding.UTF8.GetString(keyBuffer, keySize - 1); // Exclude null terminator
                    }

                    // Get value handle. The C-API returns a new handle that we own.
                    IntPtr valueHandle = YiniManager.YiniMap_GetValueAt(Handle, i);
                    if (valueHandle == IntPtr.Zero) continue;

                    list.Add(new KeyValuePair<string, YiniValue>(key, new YiniValue(valueHandle)));
                }
                finally
                {
                    if (rentedBuffer != null)
                    {
                        ArrayPool<byte>.Shared.Return(rentedBuffer);
                    }
                }
            }
            return list;
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
    public unsafe partial class YiniManager : IDisposable
    {
        private const string LibName = "Yini"; // Assumes libYini.so or Yini.dll
        private IntPtr _managerPtr;
        private bool _disposed = false;

        #region PInvoke Signatures
        // Manager
        [LibraryImport(LibName, EntryPoint = "yini_manager_create")]
        internal static partial IntPtr YiniManager_Create();

        [LibraryImport(LibName, EntryPoint = "yini_manager_destroy")]
        internal static partial void YiniManager_Destroy(IntPtr manager);

        [LibraryImport(LibName, EntryPoint = "yini_manager_load")]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static partial bool YiniManager_Load(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string filepath);

        [LibraryImport(LibName, EntryPoint = "yini_manager_load_from_string")]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static partial bool YiniManager_LoadFromString(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string content, [MarshalAs(UnmanagedType.LPStr)] string virtualFilepath);

        [LibraryImport(LibName, EntryPoint = "yini_manager_save_changes")]
        internal static partial void YiniManager_SaveChanges(IntPtr manager);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_value")]
        internal static partial IntPtr YiniManager_GetValue(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string section, [MarshalAs(UnmanagedType.LPStr)] string key);

        [LibraryImport(LibName, EntryPoint = "yini_manager_set_value")]
        internal static partial void YiniManager_SetValue(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string section, [MarshalAs(UnmanagedType.LPStr)] string key, IntPtr valueHandle);

        [LibraryImport(LibName, EntryPoint = "yini_manager_has_key")]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static partial bool YiniManager_HasKey(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string section, [MarshalAs(UnmanagedType.LPStr)] string key);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_last_error")]
        internal static partial int YiniManager_GetLastError(IntPtr manager, byte* outBuffer, int bufferSize);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_macro_count")]
        internal static partial int YiniManager_GetMacroCount(IntPtr manager);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_macro_name_at")]
        internal static partial int YiniManager_GetMacroNameAt(IntPtr manager, int index, byte* outBuffer, int bufferSize);

        // Value Handles
        [LibraryImport(LibName, EntryPoint = "yini_value_destroy")]
        internal static partial void YiniValue_Destroy(IntPtr handle);

        [LibraryImport(LibName, EntryPoint = "yini_value_get_type")]
        internal static partial YiniValueType YiniValue_GetType(IntPtr handle);

        // Create Value Handles
        [LibraryImport(LibName, EntryPoint = "yini_value_create_double")]
        internal static partial IntPtr YiniValue_CreateDouble(double value);

        [LibraryImport(LibName, EntryPoint = "yini_value_create_string")]
        internal static partial IntPtr YiniValue_CreateString([MarshalAs(UnmanagedType.LPStr)] string value);

        [LibraryImport(LibName, EntryPoint = "yini_value_create_bool")]
        internal static partial IntPtr YiniValue_CreateBool([MarshalAs(UnmanagedType.I1)] bool value);

        [LibraryImport(LibName, EntryPoint = "yini_value_create_array")]
        internal static partial IntPtr YiniValue_CreateArray();

        [LibraryImport(LibName, EntryPoint = "yini_value_create_map")]
        internal static partial IntPtr YiniValue_CreateMap();

        // Get Data from Value Handles
        [LibraryImport(LibName, EntryPoint = "yini_value_get_double")]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static partial bool YiniValue_GetDouble(IntPtr handle, out double outValue);

        [LibraryImport(LibName, EntryPoint = "yini_value_get_string")]
        internal static partial int YiniValue_GetString(IntPtr handle, byte* outBuffer, int bufferSize);

        [LibraryImport(LibName, EntryPoint = "yini_value_get_bool")]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static partial bool YiniValue_GetBool(IntPtr handle, [MarshalAs(UnmanagedType.I1)] out bool outValue);

        [LibraryImport(LibName, EntryPoint = "yini_value_get_dyna_value")]
        internal static partial IntPtr YiniValue_GetDynaValue(IntPtr handle);

        // Array Manipulation
        [LibraryImport(LibName, EntryPoint = "yini_array_get_size")]
        internal static partial int YiniArray_GetSize(IntPtr handle);

        [LibraryImport(LibName, EntryPoint = "yini_array_get_element")]
        internal static partial IntPtr YiniArray_GetElement(IntPtr handle, int index);

        [LibraryImport(LibName, EntryPoint = "yini_array_add_element")]
        internal static partial void YiniArray_AddElement(IntPtr arrayHandle, IntPtr elementHandle);

        // Map Manipulation
        [LibraryImport(LibName, EntryPoint = "yini_map_get_size")]
        internal static partial int YiniMap_GetSize(IntPtr handle);

        [LibraryImport(LibName, EntryPoint = "yini_map_get_value_at")]
        internal static partial IntPtr YiniMap_GetValueAt(IntPtr handle, int index);

        [LibraryImport(LibName, EntryPoint = "yini_map_get_key_at")]
        internal static partial int YiniMap_GetKeyAt(IntPtr handle, int index, byte* outBuffer, int bufferSize);

        [LibraryImport(LibName, EntryPoint = "yini_map_set_value")]
        internal static partial void YiniMap_SetValue(IntPtr mapHandle, [MarshalAs(UnmanagedType.LPStr)] string key, IntPtr valueHandle);

        // Schema Validation
        [LibraryImport(LibName, EntryPoint = "yini_manager_validate")]
        internal static partial int YiniManager_Validate(IntPtr manager);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_validation_error")]
        internal static partial int YiniManager_GetValidationError(IntPtr manager, int index, byte* outBuffer, int bufferSize);

        [LibraryImport(LibName, EntryPoint = "yini_manager_find_key_at_pos")]
        internal static partial int YiniManager_FindKeyAtPos(IntPtr manager, int line, int column, byte* outSection, ref int sectionSize, byte* outKey, ref int keySize);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_definition_location")]
        [return: MarshalAs(UnmanagedType.I1)]
        internal static partial bool YiniManager_GetDefinitionLocation(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string? sectionName, [MarshalAs(UnmanagedType.LPStr)] string symbolName, byte* outFilePath, ref int filePathSize, out int outLine, out int outColumn);

        // Schema Functions
        [LibraryImport(LibName, EntryPoint = "yini_manager_get_schema_section_count")]
        internal static partial int YiniManager_GetSchemaSectionCount(IntPtr manager);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_schema_section_name_at")]
        internal static partial int YiniManager_GetSchemaSectionNameAt(IntPtr manager, int index, byte* outBuffer, int bufferSize);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_schema_key_count")]
        internal static partial int YiniManager_GetSchemaKeyCount(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string sectionName);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_schema_key_details_at")]
        internal static partial int YiniManager_GetSchemaKeyDetailsAt(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string sectionName, int index, byte* outKeyName, ref int keyNameSize, byte* outTypeName, ref int typeNameSize, [MarshalAs(UnmanagedType.I1)] out bool outIsRequired);

        // Resolved Data Iteration
        [LibraryImport(LibName, EntryPoint = "yini_manager_get_resolved_section_count")]
        internal static partial int YiniManager_GetResolvedSectionCount(IntPtr manager);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_resolved_section_name_at")]
        internal static partial int YiniManager_GetResolvedSectionNameAt(IntPtr manager, int index, byte* outBuffer, int bufferSize);

        [LibraryImport(LibName, EntryPoint = "yini_manager_get_section_as_map")]
        internal static partial IntPtr YiniManager_GetSectionAsMap(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string sectionName);
        #endregion

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniManager"/> class.
        /// </summary>
        /// <exception cref="YiniException">Thrown if the native YINI manager cannot be created.</exception>
        public YiniManager()
        {
            _managerPtr = YiniManager_Create();
            if (_managerPtr == IntPtr.Zero)
            {
                // This is a catastrophic failure, as we can't even get an error message from the manager.
                throw new YiniException("Failed to create the native YiniManager instance. The native library may be missing or corrupted.");
            }
        }

        private void CheckForError()
        {
            int errorSize = YiniManager_GetLastError(_managerPtr, null, 0);
            if (errorSize > 0)
            {
                byte[]? rentedBuffer = null;
                try
                {
                    rentedBuffer = ArrayPool<byte>.Shared.Rent(errorSize);
                    fixed (byte* errorBuffer = rentedBuffer)
                    {
                        YiniManager_GetLastError(_managerPtr, errorBuffer, errorSize);
                        throw new YiniException(Encoding.UTF8.GetString(errorBuffer, errorSize - 1));
                    }
                }
                finally
                {
                    if (rentedBuffer != null)
                    {
                        ArrayPool<byte>.Shared.Return(rentedBuffer);
                    }
                }
            }
        }

        /// <summary>
        /// Loads and parses a YINI file from the specified path.
        /// </summary>
        /// <param name="filepath">The path to the YINI file.</param>
        /// <exception cref="YiniException">Thrown if the file fails to load for any reason (e.g., not found, parse error).</exception>
        public void Load(string filepath)
        {
            if (!YiniManager_Load(_managerPtr, filepath))
            {
                CheckForError();
            }
        }

        /// <summary>
        /// Loads and parses a YINI configuration from a string.
        /// </summary>
        /// <param name="content">The string content to parse.</param>
        /// <param name="virtualFilepath">A virtual path to use for error reporting. Defaults to "string".</param>
        /// <exception cref="YiniException">Thrown if the string content fails to parse.</exception>
        public void LoadFromString(string content, string virtualFilepath = "string")
        {
            if (!YiniManager_LoadFromString(_managerPtr, content, virtualFilepath))
            {
                CheckForError();
            }
        }

        /// <summary>
        /// Saves any changes made to dynamic values back to the original file.
        /// </summary>
        /// <exception cref="YiniException">Thrown if the changes fail to save.</exception>
        public void SaveChanges()
        {
            YiniManager_SaveChanges(_managerPtr);
            CheckForError();
        }

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
            if (valueHandle == IntPtr.Zero)
            {
                // Clear any potential "key not found" error, as returning null is the expected contract here.
                YiniManager_GetLastError(_managerPtr, null, 0);
                return null;
            }
            return new YiniValue(valueHandle);
        }

        /// <summary>
        /// Checks if a key exists in a given section.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <returns>true if the key exists; otherwise, false.</returns>
        public bool HasKey(string section, string key)
        {
            return YiniManager_HasKey(_managerPtr, section, key);
        }

        /// <summary>
        /// Sets a value for a specific key in a section.
        /// </summary>
        /// <param name="section">The name of the section.</param>
        /// <param name="key">The name of the key.</param>
        /// <param name="value">The <see cref="YiniValue"/> to set. The value is copied, so the caller retains ownership of the passed instance.</param>
        /// <exception cref="YiniException">Thrown if the value fails to be set.</exception>
        public void SetValue(string section, string key, YiniValue value)
        {
            // The C-API copies the value, we still own our 'value' handle.
            YiniManager_SetValue(_managerPtr, section, key, value.Handle);
            CheckForError();
        }

        /// <summary>
        /// Gets a list of all defined macro names.
        /// </summary>
        /// <returns>A list of macro names.</returns>
        public List<string> GetMacroNames()
        {
            var names = new List<string>();
            int count = YiniManager_GetMacroCount(_managerPtr);
            for (int i = 0; i < count; i++)
            {
                int nameSize = YiniManager_GetMacroNameAt(_managerPtr, i, null, 0);
                if (nameSize > 0)
                {
                    byte[]? rentedBuffer = null;
                    try
                    {
                        rentedBuffer = ArrayPool<byte>.Shared.Rent(nameSize);
                        fixed (byte* buffer = rentedBuffer)
                        {
                            YiniManager_GetMacroNameAt(_managerPtr, i, buffer, nameSize);
                            names.Add(Encoding.UTF8.GetString(buffer, nameSize - 1));
                        }
                    }
                    finally
                    {
                        if (rentedBuffer != null)
                        {
                            ArrayPool<byte>.Shared.Return(rentedBuffer);
                        }
                    }
                }
            }
            return names;
        }

        /// <summary>
        /// Gets a value converted to a specified type for a given section and key.
        /// This is the recommended method for retrieving values.
        /// </summary>
        /// <typeparam name="T">The type to convert the value to.</typeparam>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="defaultValue">The value to return if the key is not found or conversion fails.</param>
        /// <returns>The converted value or the default value.</returns>
        public T? Get<T>(string section, string key, T? defaultValue = default)
        {
            using (var yiniValue = GetValue(section, key))
            {
                if (yiniValue == null)
                {
                    return defaultValue;
                }

                try
                {
                    object? convertedValue = ConvertYiniValue(yiniValue, typeof(T));
                    if (convertedValue is T result)
                    {
                        return result;
                    }
                }
                catch (Exception ex) when (ex is InvalidCastException || ex is FormatException)
                {
                    // Conversion failed, fall through to return default value.
                    // This is useful if, for example, a string value cannot be parsed into a Guid.
                }

                return defaultValue;
            }
        }

        // --- Convenience methods for primitive types ---

        /// <summary>
        /// Gets a double value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="defaultValue">The value to return if the key is not found or is not a double.</param>
        /// <param name="yiniValue">An optional, pre-fetched YiniValue to use, avoiding a redundant lookup.</param>
        /// <returns>The double value or the default value.</returns>
        public double GetDouble(string? section, string? key, double defaultValue = 0.0, YiniValue? yiniValue = null)
        {
            bool valueWasPassed = yiniValue != null;
            if (!valueWasPassed)
            {
                if (section == null || key == null) return defaultValue;
                yiniValue = GetValue(section, key);
            }

            try
            {
                if (yiniValue != null && yiniValue.Type == YiniValueType.Double)
                {
                    return yiniValue.AsDouble();
                }
                return defaultValue;
            }
            finally
            {
                if (!valueWasPassed && yiniValue != null)
                {
                    yiniValue.Dispose();
                }
            }
        }

        /// <summary>
        /// Gets a string value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="defaultValue">The value to return if the key is not found or is not a string.</param>
        /// <param name="yiniValue">An optional, pre-fetched YiniValue to use, avoiding a redundant lookup.</param>
        /// <returns>The string value or the default value.</returns>
        public string GetString(string? section, string? key, string defaultValue = "", YiniValue? yiniValue = null)
        {
            bool valueWasPassed = yiniValue != null;
            if (!valueWasPassed)
            {
                if (section == null || key == null) return defaultValue;
                yiniValue = GetValue(section, key);
            }

            try
            {
                if (yiniValue != null && yiniValue.Type == YiniValueType.String)
                {
                    return yiniValue.AsString();
                }
                return defaultValue;
            }
            finally
            {
                if (!valueWasPassed && yiniValue != null)
                {
                    yiniValue.Dispose();
                }
            }
        }

        /// <summary>
        /// Gets a boolean value for a given section and key.
        /// </summary>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="defaultValue">The value to return if the key is not found or is not a boolean.</param>
        /// <param name="yiniValue">An optional, pre-fetched YiniValue to use, avoiding a redundant lookup.</param>
        /// <returns>The boolean value or the default value.</returns>
        public bool GetBool(string? section, string? key, bool defaultValue = false, YiniValue? yiniValue = null)
        {
            bool valueWasPassed = yiniValue != null;
            if (!valueWasPassed)
            {
                if (section == null || key == null) return defaultValue;
                yiniValue = GetValue(section, key);
            }

            try
            {
                if (yiniValue != null && yiniValue.Type == YiniValueType.Bool)
                {
                    return yiniValue.AsBool();
                }
                return defaultValue;
            }
            finally
            {
                if (!valueWasPassed && yiniValue != null)
                {
                    yiniValue.Dispose();
                }
            }
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
                string key = keyAttribute?.Key ?? ToSnakeCase(prop.Name);
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

        /// <summary>
        /// Saves the properties of a C# object to the specified YINI section.
        /// This enables two-way data binding, writing the object's state back to the YINI manager.
        /// The changes are marked as dirty and can be persisted by calling <see cref="SaveChanges()"/>.
        /// </summary>
        /// <typeparam name="T">The type of the object to save.</typeparam>
        /// <param name="section">The name of the section to save the data to.</param>
        /// <param name="instance">The instance of the object whose properties will be saved.</param>
        public void SaveChanges<T>(string section, T instance) where T : class
        {
            if (instance == null) return;

            var properties = typeof(T).GetProperties(BindingFlags.Public | BindingFlags.Instance);

            foreach (var prop in properties)
            {
                if (!prop.CanRead) continue;

                var keyAttribute = prop.GetCustomAttribute<YiniKeyAttribute>();
                string key = keyAttribute?.Key ?? ToSnakeCase(prop.Name);

                object? propValue = prop.GetValue(instance);
                if (propValue == null) continue;

                using (var yiniValue = ConvertToYiniValue(propValue))
                {
                    if (yiniValue != null)
                    {
                        SetValue(section, key, yiniValue);
                    }
                }
            }
        }

        private YiniValue? ConvertToYiniValue(object? value)
        {
            if (value == null) return null;

            var type = value.GetType();

            if (type == typeof(string)) return YiniValue.Create((string)value);
            if (type == typeof(bool)) return YiniValue.Create((bool)value);
            if (type == typeof(int)) return YiniValue.Create((double)(int)value);
            if (type == typeof(float)) return YiniValue.Create((double)(float)value);
            if (type == typeof(double)) return YiniValue.Create((double)value);

            if (value is IList list)
            {
                var yiniArray = YiniValue.CreateArray();
                foreach (var item in list)
                {
                    using (var elementValue = ConvertToYiniValue(item))
                    {
                        if (elementValue != null)
                        {
                            yiniArray.AddArrayElement(elementValue);
                        }
                    }
                }
                return yiniArray;
            }

            if (value is IDictionary dict)
            {
                var yiniMap = YiniValue.CreateMap();
                foreach (var key in dict.Keys)
                {
                    if (key is string stringKey)
                    {
                        using (var elementValue = ConvertToYiniValue(dict[key]))
                        {
                            if (elementValue != null)
                            {
                                yiniMap.SetMapValue(stringKey, elementValue);
                            }
                        }
                    }
                }
                return yiniMap;
            }

            return null;
        }

        private string ToSnakeCase(string text)
        {
            if (string.IsNullOrEmpty(text)) { return text; }

            var sb = new StringBuilder();
            sb.Append(char.ToLowerInvariant(text[0]));
            for (int i = 1; i < text.Length; ++i)
            {
                char c = text[i];
                if (char.IsUpper(c))
                {
                    sb.Append('_');
                    sb.Append(char.ToLowerInvariant(c));
                }
                else
                {
                    sb.Append(c);
                }
            }
            return sb.ToString();
        }

        private object? ConvertYiniValue(YiniValue yiniValue, Type targetType)
        {
            try
            {
                switch (yiniValue.Type)
                {
                    case YiniValueType.Double:
                        return Convert.ChangeType(yiniValue.AsDouble(), targetType);
                    case YiniValueType.String:
                        // Special case for Guid
                        if (targetType == typeof(Guid))
                            return Guid.Parse(yiniValue.AsString());
                        if (targetType == typeof(string))
                            return yiniValue.AsString();
                        // This will throw if the string cannot be converted, which is caught below
                        return Convert.ChangeType(yiniValue.AsString(), targetType);
                    case YiniValueType.Bool:
                        return Convert.ChangeType(yiniValue.AsBool(), targetType);
                case YiniValueType.Array:
                    return ConvertYiniArray(yiniValue, targetType);
                case YiniValueType.Map:
                    // Check for special struct types before treating as a generic dictionary
                    if (targetType == typeof(Vector2)) return ConvertYiniMapToVector2(yiniValue);
                    if (targetType == typeof(Vector3)) return ConvertYiniMapToVector3(yiniValue);
                    if (targetType == typeof(Vector4)) return ConvertYiniMapToVector4(yiniValue);
                    if (targetType == typeof(Color)) return ConvertYiniMapToColor(yiniValue);
                    return ConvertYiniMap(yiniValue, targetType);
                case YiniValueType.Dyna:
                    using (var innerValue = yiniValue.AsDynaValue())
                    {
                        return ConvertYiniValue(innerValue, targetType);
                    }
                default:
                    return null;
            }
        }
        catch (Exception ex) when (ex is InvalidCastException || ex is FormatException)
        {
            throw new FormatException($"Cannot convert YINI value of type {yiniValue.Type} to target type {targetType.Name}", ex);
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

        private object? ConvertYiniMapToVector2(YiniValue yiniValue)
        {
            var map = yiniValue.AsMap();
            try
            {
                var dict = new Dictionary<string, double>();
                foreach (var kvp in map)
                {
                    if (kvp.Value.Type == YiniValueType.Double)
                        dict[kvp.Key] = kvp.Value.AsDouble();
                }

                if (dict.TryGetValue("x", out var x) && dict.TryGetValue("y", out var y))
                {
                    return new Vector2((float)x, (float)y);
                }
                return null;
            }
            finally
            {
                foreach (var kvp in map) kvp.Value.Dispose();
            }
        }

        private object? ConvertYiniMapToVector3(YiniValue yiniValue)
        {
            var map = yiniValue.AsMap();
            try
            {
                var dict = new Dictionary<string, double>();
                foreach (var kvp in map)
                {
                    if (kvp.Value.Type == YiniValueType.Double)
                        dict[kvp.Key] = kvp.Value.AsDouble();
                }

                if (dict.TryGetValue("x", out var x) && dict.TryGetValue("y", out var y) && dict.TryGetValue("z", out var z))
                {
                    return new Vector3((float)x, (float)y, (float)z);
                }
                return null;
            }
            finally
            {
                foreach (var kvp in map) kvp.Value.Dispose();
            }
        }

        private object? ConvertYiniMapToVector4(YiniValue yiniValue)
        {
            var map = yiniValue.AsMap();
            try
            {
                var dict = new Dictionary<string, double>();
                foreach (var kvp in map)
                {
                    if (kvp.Value.Type == YiniValueType.Double)
                        dict[kvp.Key] = kvp.Value.AsDouble();
                }

                if (dict.TryGetValue("x", out var x) && dict.TryGetValue("y", out var y) && dict.TryGetValue("z", out var z) && dict.TryGetValue("w", out var w))
                {
                    return new Vector4((float)x, (float)y, (float)z, (float)w);
                }
                return null;
            }
            finally
            {
                foreach (var kvp in map) kvp.Value.Dispose();
            }
        }

        private object? ConvertYiniMapToColor(YiniValue yiniValue)
        {
            var map = yiniValue.AsMap();
            try
            {
                var dict = new Dictionary<string, double>();
                foreach (var kvp in map)
                {
                    if (kvp.Value.Type == YiniValueType.Double)
                        dict[kvp.Key] = kvp.Value.AsDouble();
                }

                if (dict.TryGetValue("r", out var r) && dict.TryGetValue("g", out var g) && dict.TryGetValue("b", out var b))
                {
                    // Alpha is optional, defaults to 255
                    dict.TryGetValue("a", out var a);
                    return new Color((byte)r, (byte)g, (byte)b, (byte)(dict.ContainsKey("a") ? a : 255));
                }
                return null;
            }
            finally
            {
                foreach (var kvp in map) kvp.Value.Dispose();
            }
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
        /// <param name="yiniValue">An optional, pre-fetched YiniValue to use, avoiding a redundant lookup.</param>
        /// <returns>A new <see cref="List{T}"/> containing the elements from the YINI array, or an empty list if the key is not found or is not an array.</returns>
        public List<T> GetList<T>(string? section, string? key, YiniValue? yiniValue = null)
        {
            bool valueWasPassed = yiniValue != null;
            if (!valueWasPassed)
            {
                if (section == null || key == null) return new List<T>();
                yiniValue = GetValue(section, key);
            }

            try
            {
                if (yiniValue == null || yiniValue.Type != YiniValueType.Array)
                {
                    return new List<T>();
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
            finally
            {
                if (!valueWasPassed && yiniValue != null)
                {
                    yiniValue.Dispose();
                }
            }
        }

        /// <summary>
        /// Gets a dictionary from a map value in a YINI section.
        /// </summary>
        /// <typeparam name="T">The value type of the dictionary.</typeparam>
        /// <param name="section">The section name.</param>
        /// <param name="key">The key name.</param>
        /// <param name="yiniValue">An optional, pre-fetched YiniValue to use, avoiding a redundant lookup.</param>
        /// <returns>A new <see cref="Dictionary{TKey, TValue}"/> with string keys, or an empty dictionary if the key is not found or is not a map.</returns>
        public unsafe Dictionary<string, T> GetDictionary<T>(string? section, string? key, YiniValue? yiniValue = null)
        {
            bool valueWasPassed = yiniValue != null;
            if (!valueWasPassed)
            {
                if (section == null || key == null) return new Dictionary<string, T>();
                yiniValue = GetValue(section, key);
            }

            try
            {
                if (yiniValue == null || yiniValue.Type != YiniValueType.Map)
                {
                    return new Dictionary<string, T>();
                }

                var dict = new Dictionary<string, T>();
                var valueType = typeof(T);
                int size = yiniValue.MapSize;

                for (int i = 0; i < size; i++)
                {
                    int keySize = YiniMap_GetKeyAt(yiniValue.Handle, i, null, 0);
                    if (keySize <= 0) continue;

                    byte[]? keyRented = null;
                    string mapKey;
                    try
                    {
                        keyRented = ArrayPool<byte>.Shared.Rent(keySize);
                        fixed (byte* keyBuffer = keyRented)
                        {
                            YiniMap_GetKeyAt(yiniValue.Handle, i, keyBuffer, keySize);
                            mapKey = Encoding.UTF8.GetString(keyBuffer, keySize - 1);
                        }
                    }
                    finally
                    {
                        if (keyRented != null) ArrayPool<byte>.Shared.Return(keyRented);
                    }

                    IntPtr valueHandle = YiniMap_GetValueAt(yiniValue.Handle, i);
                    if (valueHandle == IntPtr.Zero) continue;

                    using (var element = new YiniValue(valueHandle))
                    {
                        var converted = ConvertYiniValue(element, valueType);
                        if (converted != null)
                        {
                            dict[mapKey] = (T)converted;
                        }
                    }
                }
                return dict;
            }
            finally
            {
                if (!valueWasPassed && yiniValue != null)
                {
                    yiniValue.Dispose();
                }
            }
        }

        #region Iteration
        /// <summary>
        /// Gets a list of all section names that have been resolved.
        /// </summary>
        /// <returns>A list of section names.</returns>
        public unsafe List<string> GetResolvedSectionNames()
        {
            var names = new List<string>();
            int count = YiniManager_GetResolvedSectionCount(_managerPtr);
            if (count <= 0) return names;

            for (int i = 0; i < count; i++)
            {
                int nameSize = YiniManager_GetResolvedSectionNameAt(_managerPtr, i, null, 0);
                if (nameSize > 0)
                {
                    byte[]? rentedBuffer = null;
                    try
                    {
                        rentedBuffer = ArrayPool<byte>.Shared.Rent(nameSize);
                        fixed (byte* buffer = rentedBuffer)
                        {
                            YiniManager_GetResolvedSectionNameAt(_managerPtr, i, buffer, nameSize);
                            names.Add(Encoding.UTF8.GetString(buffer, nameSize - 1));
                        }
                    }
                    finally
                    {
                        if (rentedBuffer != null)
                        {
                            ArrayPool<byte>.Shared.Return(rentedBuffer);
                        }
                    }
                }
            }
            return names;
        }

        /// <summary>
        /// Gets all key-value pairs for a given section as a dictionary.
        /// </summary>
        /// <param name="sectionName">The name of the section.</param>
        /// <returns>A dictionary containing all keys and their string representations, or null if the section doesn't exist.</returns>
        public Dictionary<string, string?>? GetSectionAsDictionary(string sectionName)
        {
            var mapHandle = YiniManager_GetSectionAsMap(_managerPtr, sectionName);
            if (mapHandle == IntPtr.Zero)
            {
                return null;
            }

            using var mapValue = new YiniValue(mapHandle);
            var dict = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);

            var mapPairs = mapValue.AsMap();
            try
            {
                foreach (var kvp in mapPairs)
                {
                    // For configuration, we need the string representation of the value.
                    dict[kvp.Key] = StringifyValue(kvp.Value);
                }
                return dict;
            }
            finally
            {
                // Ensure the inner YiniValue handles are disposed
                foreach (var kvp in mapPairs)
                {
                    kvp.Value.Dispose();
                }
            }
        }
        #endregion

        #region Schema
        /// <summary>
        /// Gets a list of all section names defined in the loaded schema.
        /// </summary>
        /// <returns>A list of section names, or an empty list if no schema is loaded.</returns>
        public unsafe List<string> GetSchemaSectionNames()
        {
            var names = new List<string>();
            int count = YiniManager_GetSchemaSectionCount(_managerPtr);
            if (count <= 0) return names;

            for (int i = 0; i < count; i++)
            {
                int nameSize = YiniManager_GetSchemaSectionNameAt(_managerPtr, i, null, 0);
                if (nameSize > 0)
                {
                    byte[]? rentedBuffer = null;
                    try
                    {
                        rentedBuffer = ArrayPool<byte>.Shared.Rent(nameSize);
                        fixed (byte* buffer = rentedBuffer)
                        {
                            YiniManager_GetSchemaSectionNameAt(_managerPtr, i, buffer, nameSize);
                            names.Add(Encoding.UTF8.GetString(buffer, nameSize - 1));
                        }
                    }
                    finally
                    {
                        if (rentedBuffer != null)
                        {
                            ArrayPool<byte>.Shared.Return(rentedBuffer);
                        }
                    }
                }
            }
            return names;
        }

        /// <summary>
        /// Gets a list of all key definitions for a given section within the loaded schema.
        /// </summary>
        /// <param name="sectionName">The name of the section to query.</param>
        /// <returns>A list of <see cref="SchemaKeyInfo"/> objects, or an empty list if the section is not found in the schema.</returns>
        public unsafe List<SchemaKeyInfo> GetSchemaKeysForSection(string sectionName)
        {
            var keys = new List<SchemaKeyInfo>();
            int count = YiniManager_GetSchemaKeyCount(_managerPtr, sectionName);
            if (count <= 0) return keys;

            for (int i = 0; i < count; i++)
            {
                int keyNameSize = 0;
                int typeNameSize = 0;

                // First call to get buffer sizes
                YiniManager_GetSchemaKeyDetailsAt(_managerPtr, sectionName, i, null, ref keyNameSize, null, ref typeNameSize, out _);

                if (keyNameSize > 0 && typeNameSize > 0)
                {
                    byte[]? keyRented = null;
                    byte[]? typeRented = null;
                    try
                    {
                        keyRented = ArrayPool<byte>.Shared.Rent(keyNameSize);
                        typeRented = ArrayPool<byte>.Shared.Rent(typeNameSize);

                        fixed (byte* keyBuffer = keyRented)
                        fixed (byte* typeBuffer = typeRented)
                        {
                            if (YiniManager_GetSchemaKeyDetailsAt(_managerPtr, sectionName, i, keyBuffer, ref keyNameSize, typeBuffer, ref typeNameSize, out bool isRequired) == 1)
                            {
                                keys.Add(new SchemaKeyInfo
                                {
                                    Name = Encoding.UTF8.GetString(keyBuffer, keyNameSize - 1),
                                    TypeName = Encoding.UTF8.GetString(typeBuffer, typeNameSize - 1),
                                    IsRequired = isRequired
                                });
                            }
                        }
                    }
                    finally
                    {
                        if (keyRented != null) ArrayPool<byte>.Shared.Return(keyRented);
                        if (typeRented != null) ArrayPool<byte>.Shared.Return(typeRented);
                    }
                }
            }
            return keys;
        }
        #endregion

        /// <summary>
        /// Validates the loaded YINI data against its defined schema.
        /// </summary>
        /// <returns>A list of validation error messages. An empty list indicates that validation passed.</returns>
        public unsafe List<string> Validate()
        {
            var errors = new List<string>();
            int errorCount = YiniManager_Validate(_managerPtr);

            if (errorCount <= 0)
            {
                return errors; // No errors or an issue with validation itself.
            }

            for (int i = 0; i < errorCount; i++)
            {
                int requiredSize = YiniManager_GetValidationError(_managerPtr, i, null, 0);
                if (requiredSize <= 0) continue;

                byte[]? rentedBuffer = null;
                try
                {
                    rentedBuffer = ArrayPool<byte>.Shared.Rent(requiredSize);
                    fixed (byte* buffer = rentedBuffer)
                    {
                        YiniManager_GetValidationError(_managerPtr, i, buffer, requiredSize);
                        errors.Add(Encoding.UTF8.GetString(buffer, requiredSize - 1));
                    }
                }
                finally
                {
                    if (rentedBuffer != null)
                    {
                        ArrayPool<byte>.Shared.Return(rentedBuffer);
                    }
                }
            }

            return errors;
        }

        /// <summary>
        /// Finds the section and key name for a symbol at a specific line and column in the source file.
        /// This is primarily used for IDE features like "Go to Definition".
        /// </summary>
        /// <param name="line">The line number (1-based).</param>
        /// <param name="column">The column number (1-based).</param>
        /// <returns>A tuple containing the section and key name if a symbol is found; otherwise, null.</returns>
        public unsafe (string Section, string Key)? FindKeyAtPos(int line, int column)
        {
            int sectionSize = 0;
            int keySize = 0;

            if (YiniManager_FindKeyAtPos(_managerPtr, line, column, null, ref sectionSize, null, ref keySize) == 0)
            {
                return null;
            }

            byte[]? sectionRented = null;
            byte[]? keyRented = null;
            try
            {
                sectionRented = ArrayPool<byte>.Shared.Rent(sectionSize);
                keyRented = ArrayPool<byte>.Shared.Rent(keySize);

                string section, key;
                fixed (byte* sectionBuffer = sectionRented)
                fixed (byte* keyBuffer = keyRented)
                {
                    if (YiniManager_FindKeyAtPos(_managerPtr, line, column, sectionBuffer, ref sectionSize, keyBuffer, ref keySize) == 0)
                    {
                        return null;
                    }
                    section = Encoding.UTF8.GetString(sectionBuffer, sectionSize - 1);
                    key = Encoding.UTF8.GetString(keyBuffer, keySize - 1);
                }
                return (section, key);
            }
            finally
            {
                if (sectionRented != null) ArrayPool<byte>.Shared.Return(sectionRented);
                if (keyRented != null) ArrayPool<byte>.Shared.Return(keyRented);
            }
        }

        /// <summary>
        /// Gets the location (file path, line, and column) of a symbol's definition.
        /// This is used for IDE features like "Go to Definition" to navigate to where a macro or inherited section is defined.
        /// </summary>
        /// <param name="sectionName">The name of the section where the symbol is referenced. Can be null for global symbols like macros.</param>
        /// <param name="symbolName">The name of the symbol to find (e.g., "@macroName" or "ParentSection").</param>
        /// <returns>A tuple containing the file path, line, and column of the definition if found; otherwise, null.</returns>
        public unsafe (string FilePath, int Line, int Column)? GetDefinitionLocation(string? sectionName, string symbolName)
        {
            int filePathSize = 0;
            if (!YiniManager_GetDefinitionLocation(_managerPtr, sectionName, symbolName, null, ref filePathSize, out int line, out int column))
            {
                return null;
            }

            byte[]? rentedBuffer = null;
            try
            {
                rentedBuffer = ArrayPool<byte>.Shared.Rent(filePathSize);
                string filePath;
                fixed (byte* buffer = rentedBuffer)
                {
                    if (!YiniManager_GetDefinitionLocation(_managerPtr, sectionName, symbolName, buffer, ref filePathSize, out line, out column))
                    {
                        return null;
                    }
                    filePath = Encoding.UTF8.GetString(buffer, filePathSize - 1);
                }
                return (filePath, line, column);
            }
            finally
            {
                if (rentedBuffer != null)
                {
                    ArrayPool<byte>.Shared.Return(rentedBuffer);
                }
            }
        }

        /// <summary>
        /// Provides a user-friendly string representation of a YiniValue.
        /// </summary>
        /// <param name="value">The YiniValue to stringify.</param>
        /// <returns>A string representation of the value.</returns>
        public string StringifyValue(YiniValue value)
        {
            switch (value.Type)
            {
                case YiniValueType.Double: return value.AsDouble().ToString();
                case YiniValueType.Bool: return value.AsBool().ToString().ToLower();
                case YiniValueType.String: return $"\"{value.AsString()}\"";
                case YiniValueType.Array:
                    var items = new List<string>();
                    for (int i = 0; i < value.ArraySize; i++) { using (var e = value.GetArrayElement(i)) items.Add(StringifyValue(e)); }
                    return $"[{string.Join(", ", items)}]";
                case YiniValueType.Map:
                    var mapPairs = value.AsMap();
                    try
                    {
                        var map = mapPairs.ToDictionary(kvp => kvp.Key, kvp => kvp.Value);
                        var pairStrings = map.Select(kvp => $"\"{kvp.Key}\": {StringifyValue(kvp.Value)}");
                        return $"{{{string.Join(", ", pairStrings)}}}";
                    }
                    finally { foreach (var kvp in mapPairs) kvp.Value.Dispose(); }
                case YiniValueType.Dyna:
                    using (var inner = value.AsDynaValue()) return $"Dyna({StringifyValue(inner)})";
                default: return "null";
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
