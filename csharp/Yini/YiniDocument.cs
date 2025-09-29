using System;
using System.Runtime.InteropServices;
using System.Text;

namespace YINI
{
    public class YiniDocument : IDisposable
    {
        private YiniDocumentHandle _handle;

        private YiniDocument(YiniDocumentHandle handle)
        {
            _handle = handle;
        }

        public static YiniDocument Parse(string content)
        {
            var errorBuffer = Marshal.AllocHGlobal(256);
            try
            {
                var handle = NativeMethods.yini_parse(content, errorBuffer, 256);
                if (handle.IsInvalid)
                {
                    var errorMessage = Marshal.PtrToStringAnsi(errorBuffer);
                    throw new InvalidOperationException($"Failed to parse YINI content: {errorMessage}");
                }
                return new YiniDocument(handle);
            }
            finally
            {
                Marshal.FreeHGlobal(errorBuffer);
            }
        }

        public YiniValue GetValue(string section, string key)
        {
            var sectionPtr = NativeMethods.yini_get_section_by_name(_handle, section);
            if (sectionPtr == IntPtr.Zero)
                throw new ArgumentException($"Section '{section}' not found.");

            var valuePtr = NativeMethods.yini_section_get_value_by_key(sectionPtr, key);
            if (valuePtr == IntPtr.Zero)
                throw new ArgumentException($"Key '{key}' not found in section '{section}'.");

            return new YiniValue(valuePtr);
        }

        public T GetValue<T>(string section, string key)
        {
            var value = GetValue(section, key);
            object result;

            switch (value.Type)
            {
                case YiniType.String:
                    result = value.AsString();
                    break;
                case YiniType.Int:
                    result = value.AsInt();
                    break;
                case YiniType.Double:
                    result = value.AsDouble();
                    break;
                case YiniType.Bool:
                    result = value.AsBool();
                    break;
                default:
                    throw new NotSupportedException($"YiniType '{value.Type}' is not supported for generic GetValue<T>.");
            }

            return (T)Convert.ChangeType(result, typeof(T));
        }

        public void SetValue<T>(string section, string key, T value)
        {
            if (value is string s)
                NativeMethods.yini_set_string_value(_handle, section, key, s);
            else if (value is int i)
                NativeMethods.yini_set_int_value(_handle, section, key, i);
            else if (value is double d)
                NativeMethods.yini_set_double_value(_handle, section, key, d);
            else if (value is bool b)
                NativeMethods.yini_set_bool_value(_handle, section, key, b);
            else
                throw new NotSupportedException($"Type '{typeof(T)}' is not supported for setting values.");
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_handle != null && !_handle.IsInvalid)
            {
                _handle.Dispose();
            }
        }
    }
}