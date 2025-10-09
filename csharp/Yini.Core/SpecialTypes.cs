using System.Runtime.InteropServices;

namespace Yini.Core
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Color
    {
        public int r;
        public int g;
        public int b;

        public override bool Equals(object obj)
        {
            if (obj is Color other)
            {
                return r == other.r && g == other.g && b == other.b;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return r.GetHashCode() ^ g.GetHashCode() ^ b.GetHashCode();
        }

        public static bool operator ==(Color left, Color right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(Color left, Color right)
        {
            return !(left == right);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Coord
    {
        public int x;
        public int y;
        public int z;

        public override bool Equals(object obj)
        {
            if (obj is Coord other)
            {
                return x == other.x && y == other.y && z == other.z;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() ^ z.GetHashCode();
        }

        public static bool operator ==(Coord left, Coord right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(Coord left, Coord right)
        {
            return !(left == right);
        }
    }
}