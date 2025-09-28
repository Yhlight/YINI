using System.Runtime.InteropServices;

namespace YINI
{
    [StructLayout(LayoutKind.Sequential)]
    public struct YiniCoord
    {
        public double X;
        public double Y;
        public double Z;
        [MarshalAs(UnmanagedType.I1)]
        public bool Is3D;

        public override string ToString() => Is3D ? $"({X}, {Y}, {Z})" : $"({X}, {Y})";
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct YiniColor
    {
        public byte R;
        public byte G;
        public byte B;

        public override string ToString() => $"#{R:X2}{G:X2}{B:X2}";
    }
}