using System;
using System.Runtime.InteropServices;
using System.Text;

namespace YINI
{
    public class YiniManager : IDisposable
    {
        private const string LibName = "yini";
        private IntPtr _handle;
        private YiniDocument? _document;
        private bool _disposed;

        [DllImport(LibName, EntryPoint = "yini_manager_create", CharSet = CharSet.Ansi)]
        private static extern IntPtr CreateInternal(string yiniFilePath);

        [DllImport(LibName, EntryPoint = "yini_manager_free")]
        private static extern void FreeInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_manager_is_loaded")]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool IsLoadedInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_manager_get_document")]
        private static extern IntPtr GetDocumentInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_manager_set_string_value", CharSet = CharSet.Ansi)]
        private static extern void SetStringValueInternal(IntPtr handle, string section, string key, string value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_int_value", CharSet = CharSet.Ansi)]
        private static extern void SetIntValueInternal(IntPtr handle, string section, string key, int value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_double_value", CharSet = CharSet.Ansi)]
        private static extern void SetDoubleValueInternal(IntPtr handle, string section, string key, double value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_bool_value", CharSet = CharSet.Ansi)]
        private static extern void SetBoolValueInternal(IntPtr handle, string section, string key, [MarshalAs(UnmanagedType.I1)] bool value);

        public YiniManager(string yiniFilePath)
        {
            _handle = CreateInternal(yiniFilePath);
            if (_handle == IntPtr.Zero || !IsLoadedInternal(_handle))
            {
                throw new InvalidOperationException($"Failed to load or create YINI manager for file: {yiniFilePath}");
            }
        }

        public bool IsLoaded => _handle != IntPtr.Zero && IsLoadedInternal(_handle);

        public YiniDocument Document
        {
            get
            {
                if (_document == null)
                {
                    IntPtr docHandle = GetDocumentInternal(_handle);
                    if (docHandle != IntPtr.Zero)
                    {
                        _document = new YiniDocument(docHandle, isOwnedByManager: true);
                    }
                }
                return _document ?? throw new InvalidOperationException("Document could not be retrieved from the manager.");
            }
        }

        public void SetValue(string section, string key, string value) => SetStringValueInternal(_handle, section, key, value);
        public void SetValue(string section, string key, int value) => SetIntValueInternal(_handle, section, key, value);
        public void SetValue(string section, string key, double value) => SetDoubleValueInternal(_handle, section, key, value);
        public void SetValue(string section, string key, bool value) => SetBoolValueInternal(_handle, section, key, value);

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                {
                    FreeInternal(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        ~YiniManager()
        {
            Dispose(false);
        }
    }
}