using System;
using System.Runtime.InteropServices;
using System.Text;

namespace YINI
{
    public class YiniDocument : IDisposable
    {
        private const string LibName = "yini";
        private IntPtr _handle;
        private bool _disposed = false;

        [DllImport(LibName, EntryPoint = "yini_parse")]
        private static extern IntPtr Parse(string content, StringBuilder errorBuffer, int errorBufferSize);

        [DllImport(LibName, EntryPoint = "yini_free_document")]
        private static extern void FreeDocument(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_section_count")]
        private static extern int GetSectionCountInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_section_by_index")]
        private static extern IntPtr GetSectionByIndexInternal(IntPtr handle, int index);

        [DllImport(LibName, EntryPoint = "yini_get_section_by_name")]
        private static extern IntPtr GetSectionByNameInternal(IntPtr handle, string name);

        public YiniDocument(string content)
        {
            var errorBuffer = new StringBuilder(1024);
            _handle = Parse(content, errorBuffer, errorBuffer.Capacity);
            if (_handle == IntPtr.Zero)
            {
                throw new InvalidOperationException($"Failed to parse YINI content: {errorBuffer.ToString()}");
            }
        }

        public int SectionCount => GetSectionCountInternal(_handle);

        public YiniSection GetSection(int index)
        {
            if (index < 0 || index >= SectionCount)
            {
                throw new ArgumentOutOfRangeException(nameof(index));
            }
            IntPtr sectionHandle = GetSectionByIndexInternal(_handle, index);
            return sectionHandle == IntPtr.Zero ? null : new YiniSection(sectionHandle);
        }

        public YiniSection GetSection(string name)
        {
            IntPtr sectionHandle = GetSectionByNameInternal(_handle, name);
            return sectionHandle == IntPtr.Zero ? null : new YiniSection(sectionHandle);
        }

        public YiniValue GetValue(string sectionName, string key)
        {
            var section = GetSection(sectionName);
            return section?.GetValue(key);
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
                    FreeDocument(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~YiniDocument()
        {
            Dispose(false);
        }
    }
}