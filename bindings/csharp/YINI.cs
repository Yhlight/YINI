using System;
using System.Runtime.InteropServices;
using System.Text;

namespace YINI
{
    /// <summary>
    /// Value types in YINI
    /// </summary>
    public enum ValueType
    {
        Nil = 0,
        Integer = 1,
        Float = 2,
        Boolean = 3,
        String = 4,
        Array = 5,
        Map = 6,
        Color = 7,
        Coord = 8
    }

    /// <summary>
    /// YINI Parser for C#
    /// </summary>
    public class Parser : IDisposable
    {
        private IntPtr handle;
        private bool disposed = false;

        // Platform-specific library name
        private const string LibraryName = "yini";

        #region P/Invoke Declarations

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_parser_create(string source);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_parser_create_from_file(string filename);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void yini_parser_destroy(IntPtr parser);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool yini_parser_parse(IntPtr parser);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_parser_get_error(IntPtr parser);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int yini_parser_get_section_count(IntPtr parser);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_parser_get_section_names(IntPtr parser, out int count);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_parser_get_section(IntPtr parser, string name);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int yini_section_get_entry_count(IntPtr section);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_section_get_keys(IntPtr section, out int count);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_section_get_value(IntPtr section, string key);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern ValueType yini_value_get_type(IntPtr value);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern long yini_value_get_integer(IntPtr value);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern double yini_value_get_float(IntPtr value);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool yini_value_get_boolean(IntPtr value);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_value_get_string(IntPtr value);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int yini_value_get_array_size(IntPtr value);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr yini_value_get_array_element(IntPtr value, int index);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void yini_free_string_array(IntPtr array, int count);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool yini_compile_to_ymeta(string input_file, string output_file);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool yini_decompile_from_ymeta(string input_file, string output_file);

        #endregion

        /// <summary>
        /// Create a parser from YINI source string
        /// </summary>
        public Parser(string source)
        {
            handle = yini_parser_create(source);
            if (handle == IntPtr.Zero)
            {
                throw new Exception("Failed to create YINI parser");
            }
        }

        /// <summary>
        /// Create a parser from a YINI file
        /// </summary>
        public static Parser FromFile(string filename)
        {
            var handle = yini_parser_create_from_file(filename);
            if (handle == IntPtr.Zero)
            {
                throw new Exception($"Failed to load YINI file: {filename}");
            }

            return new Parser(handle);
        }

        private Parser(IntPtr handle)
        {
            this.handle = handle;
        }

        /// <summary>
        /// Parse the YINI source
        /// </summary>
        public bool Parse()
        {
            CheckDisposed();
            return yini_parser_parse(handle);
        }

        /// <summary>
        /// Get last error message
        /// </summary>
        public string GetError()
        {
            CheckDisposed();
            var ptr = yini_parser_get_error(handle);
            return Marshal.PtrToStringAnsi(ptr) ?? "";
        }

        /// <summary>
        /// Get number of sections
        /// </summary>
        public int GetSectionCount()
        {
            CheckDisposed();
            return yini_parser_get_section_count(handle);
        }

        /// <summary>
        /// Get all section names
        /// </summary>
        public string[] GetSectionNames()
        {
            CheckDisposed();
            int count;
            var ptr = yini_parser_get_section_names(handle, out count);

            if (ptr == IntPtr.Zero || count == 0)
            {
                return new string[0];
            }

            string[] names = new string[count];
            IntPtr[] ptrs = new IntPtr[count];
            Marshal.Copy(ptr, ptrs, 0, count);

            for (int i = 0; i < count; i++)
            {
                names[i] = Marshal.PtrToStringAnsi(ptrs[i]) ?? "";
            }

            yini_free_string_array(ptr, count);
            return names;
        }

        /// <summary>
        /// Get a section by name
        /// </summary>
        public Section? GetSection(string name)
        {
            CheckDisposed();
            var sectionHandle = yini_parser_get_section(handle, name);
            if (sectionHandle == IntPtr.Zero)
            {
                return null;
            }

            return new Section(sectionHandle);
        }

        /// <summary>
        /// Compile a YINI file to YMETA binary format
        /// </summary>
        public static bool CompileToYMETA(string inputFile, string outputFile)
        {
            return yini_compile_to_ymeta(inputFile, outputFile);
        }

        /// <summary>
        /// Decompile a YMETA file to YINI text format
        /// </summary>
        public static bool DecompileFromYMETA(string inputFile, string outputFile)
        {
            return yini_decompile_from_ymeta(inputFile, outputFile);
        }

        private void CheckDisposed()
        {
            if (disposed)
            {
                throw new ObjectDisposedException(GetType().Name);
            }
        }

        public void Dispose()
        {
            if (!disposed)
            {
                if (handle != IntPtr.Zero)
                {
                    yini_parser_destroy(handle);
                    handle = IntPtr.Zero;
                }
                disposed = true;
            }
            GC.SuppressFinalize(this);
        }

        ~Parser()
        {
            Dispose();
        }
    }

    /// <summary>
    /// Represents a YINI section
    /// </summary>
    public class Section
    {
        private IntPtr handle;

        internal Section(IntPtr handle)
        {
            this.handle = handle;
        }

        /// <summary>
        /// Get value by key
        /// </summary>
        public Value? GetValue(string key)
        {
            var valueHandle = Parser.yini_section_get_value(handle, key);
            if (valueHandle == IntPtr.Zero)
            {
                return null;
            }

            return new Value(valueHandle);
        }

        /// <summary>
        /// Get all keys in this section
        /// </summary>
        public string[] GetKeys()
        {
            int count;
            var ptr = Parser.yini_section_get_keys(handle, out count);

            if (ptr == IntPtr.Zero || count == 0)
            {
                return new string[0];
            }

            string[] keys = new string[count];
            IntPtr[] ptrs = new IntPtr[count];
            Marshal.Copy(ptr, ptrs, 0, count);

            for (int i = 0; i < count; i++)
            {
                keys[i] = Marshal.PtrToStringAnsi(ptrs[i]) ?? "";
            }

            Parser.yini_free_string_array(ptr, count);
            return keys;
        }
    }

    /// <summary>
    /// Represents a YINI value
    /// </summary>
    public class Value
    {
        private IntPtr handle;

        internal Value(IntPtr handle)
        {
            this.handle = handle;
        }

        /// <summary>
        /// Get value type
        /// </summary>
        public ValueType GetValueType()
        {
            return Parser.yini_value_get_type(handle);
        }

        /// <summary>
        /// Get as integer
        /// </summary>
        public long AsInteger()
        {
            return Parser.yini_value_get_integer(handle);
        }

        /// <summary>
        /// Get as float
        /// </summary>
        public double AsFloat()
        {
            return Parser.yini_value_get_float(handle);
        }

        /// <summary>
        /// Get as boolean
        /// </summary>
        public bool AsBoolean()
        {
            return Parser.yini_value_get_boolean(handle);
        }

        /// <summary>
        /// Get as string
        /// </summary>
        public string AsString()
        {
            var ptr = Parser.yini_value_get_string(handle);
            return Marshal.PtrToStringAnsi(ptr) ?? "";
        }

        /// <summary>
        /// Get array size
        /// </summary>
        public int GetArraySize()
        {
            return Parser.yini_value_get_array_size(handle);
        }

        /// <summary>
        /// Get array element
        /// </summary>
        public Value? GetArrayElement(int index)
        {
            var elemHandle = Parser.yini_value_get_array_element(handle, index);
            if (elemHandle == IntPtr.Zero)
            {
                return null;
            }

            return new Value(elemHandle);
        }
    }
}
