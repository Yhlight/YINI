using System;
using System.Runtime.InteropServices;

namespace YINI
{
    public class YiniManager : IDisposable
    {
        private const string LibName = "yini";
        private IntPtr _handle;
        private YiniDocument? _document;
        private bool _disposed = false;

        [DllImport(LibName, EntryPoint = "yini_manager_create")]
        private static extern IntPtr Create(string yiniFilePath);

        [DllImport(LibName, EntryPoint = "yini_manager_free")]
        private static extern void Free(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_manager_is_loaded")]
        private static extern bool IsLoadedInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_manager_get_document")]
        private static extern IntPtr GetDocumentInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_manager_set_string_value")]
        private static extern void SetStringValueInternal(IntPtr handle, string section, string key, string value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_int_value")]
        private static extern void SetIntValueInternal(IntPtr handle, string section, string key, int value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_double_value")]
        private static extern void SetDoubleValueInternal(IntPtr handle, string section, string key, double value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_bool_value")]
        private static extern void SetBoolValueInternal(IntPtr handle, string section, string key, bool value);

        public YiniManager(string yiniFilePath)
        {
            _handle = Create(yiniFilePath);
            if (_handle == IntPtr.Zero || !IsLoadedInternal(_handle))
            {
                throw new InvalidOperationException($"Failed to load or create YINI manager for file: {yiniFilePath}");
            }
        }

        public bool IsLoaded => IsLoadedInternal(_handle);

        public YiniDocument Document
        {
            get
            {
                if (_document == null)
                {
                    IntPtr docHandle = GetDocumentInternal(_handle);
                    if (docHandle != IntPtr.Zero)
                    {
                        // The document handle from the manager is not owned by the C# wrapper,
                        // so we pass `isManaged: false` to prevent it from being freed.
                        _document = new YiniDocument(docHandle, isManaged: false);
                    }
                }
                return _document!;
            }
        }

        public void SetValue(string sectionName, string key, string value) => SetStringValueInternal(_handle, sectionName, key, value);
        public void SetValue(string sectionName, string key, int value) => SetIntValueInternal(_handle, sectionName, key, value);
        public void SetValue(string sectionName, string key, double value) => SetDoubleValueInternal(_handle, sectionName, key, value);
        public void SetValue(string sectionName, string key, bool value) => SetBoolValueInternal(_handle, sectionName, key, value);

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
                    Free(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~YiniManager()
        {
            Dispose(false);
        }
    }
}