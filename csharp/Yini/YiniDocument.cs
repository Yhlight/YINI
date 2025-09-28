using System;
using System.Runtime.InteropServices;

namespace YINI
{
    public class YiniDocument : IDisposable
    {
        private const string LibName = "yini";
        private IntPtr _handle;
        private bool _disposed = false;

        [DllImport(LibName, EntryPoint = "yini_parse")]
        private static extern IntPtr Parse(string content);

        [DllImport(LibName, EntryPoint = "yini_free_document")]
        private static extern void FreeDocument(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_section_count")]
        private static extern int GetSectionCount(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_get_section_name")]
        private static extern IntPtr GetSectionNameInternal(IntPtr handle, int sectionIndex);

        public YiniDocument(string content)
        {
            _handle = Parse(content);
            if (_handle == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to parse YINI content.");
            }
        }

        public int SectionCount => GetSectionCount(_handle);

        public string GetSectionName(int sectionIndex)
        {
            IntPtr namePtr = GetSectionNameInternal(_handle, sectionIndex);
            return Marshal.PtrToStringAnsi(namePtr);
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