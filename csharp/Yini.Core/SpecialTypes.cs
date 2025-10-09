using System;
using System.Runtime.InteropServices;

namespace Yini.Core
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Color
    {
        public int r;
        public int g;
        public int b;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Coord
    {
        public int x;
        public int y;
        public int z;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Path
    {
        /// <summary>
        /// A pointer to a C-style string (const char*).
        /// Do not access directly; use the GetValue() method.
        /// </summary>
        internal IntPtr value_ptr;

        public string GetValue()
        {
            if (value_ptr == IntPtr.Zero)
            {
                return null;
            }
            return Marshal.PtrToStringAnsi(value_ptr);
        }

        public override string ToString()
        {
            return GetValue();
        }
    }
}