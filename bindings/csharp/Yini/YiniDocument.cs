using System;

namespace Yini
{
    public class YiniDocument : IDisposable
    {
        private IntPtr handle;
        private bool disposed = false;

        public YiniDocument(string source)
        {
            handle = NativeMethods.YiniParseString(source);
            if (handle == IntPtr.Zero)
            {
                throw new Exception("Failed to parse YINI source.");
            }
        }

        ~YiniDocument()
        {
            Dispose(false);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (handle != IntPtr.Zero)
                {
                    NativeMethods.YiniFreeDocument(handle);
                    handle = IntPtr.Zero;
                }
                disposed = true;
            }
        }
    }
}