using System;
using System.Runtime.InteropServices;
using System.Text;

namespace YINI
{
    public enum YiniType
    {
        None,
        String,
        Int,
        Double,
        Bool,
        Array,
        Map,
        Dyna,
        Coord,
        Color,
        Path
    }

    public class YiniValue
    {
        private const string LibName = "yini";
        private readonly IntPtr _handle;

        [DllImport(LibName, EntryPoint = "yini_value_get_type")]
        private static extern YiniType GetTypeInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_value_get_string")]
        private static extern int GetStringInternal(IntPtr handle, StringBuilder buffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_value_get_int")]
        private static extern int GetIntInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_value_get_double")]
        private static extern double GetDoubleInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_value_get_bool")]
        private static extern bool GetBoolInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_array_get_size")]
        private static extern int GetArraySizeInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_array_get_value_by_index")]
        private static extern IntPtr GetValueByIndexInternal(IntPtr handle, int index);

        [DllImport(LibName, EntryPoint = "yini_value_get_coord")]
        private static extern void GetCoordInternal(IntPtr handle, out double x, out double y, out double z, out bool is_3d);

        [DllImport(LibName, EntryPoint = "yini_value_get_color")]
        private static extern void GetColorInternal(IntPtr handle, out byte r, out byte g, out byte b);

        [DllImport(LibName, EntryPoint = "yini_value_get_path")]
        private static extern int GetPathInternal(IntPtr handle, StringBuilder buffer, int bufferSize);

        internal YiniValue(IntPtr handle)
        {
            if (handle == IntPtr.Zero) throw new ArgumentNullException(nameof(handle));
            _handle = handle;
        }

        public YiniType Type => GetTypeInternal(_handle);

        public string AsString()
        {
            if (Type != YiniType.String) throw new InvalidCastException($"Cannot get value as string, type is {Type}.");
            int requiredSize = GetStringInternal(_handle, null, 0);
            if (requiredSize <= 0) return string.Empty;
            var sb = new StringBuilder(requiredSize);
            GetStringInternal(_handle, sb, sb.Capacity);
            return sb.ToString();
        }

        public int AsInt()
        {
            if (Type != YiniType.Int) throw new InvalidCastException($"Cannot get value as int, type is {Type}.");
            return GetIntInternal(_handle);
        }

        public double AsDouble()
        {
            if (Type != YiniType.Double) throw new InvalidCastException($"Cannot get value as double, type is {Type}.");
            return GetDoubleInternal(_handle);
        }

        public bool AsBool()
        {
            if (Type != YiniType.Bool) throw new InvalidCastException($"Cannot get value as bool, type is {Type}.");
            return GetBoolInternal(_handle);
        }

        public YiniValue[] AsArray()
        {
            if (Type != YiniType.Array) throw new InvalidCastException($"Cannot get value as array, type is {Type}.");
            int size = GetArraySizeInternal(_handle);
            var array = new YiniValue[size];
            for (int i = 0; i < size; i++)
            {
                IntPtr valueHandle = GetValueByIndexInternal(_handle, i);
                if (valueHandle != IntPtr.Zero)
                {
                    array[i] = new YiniValue(valueHandle);
                }
            }
            return array;
        }

        public YiniCoord AsCoord()
        {
            if (Type != YiniType.Coord) throw new InvalidCastException($"Cannot get value as Coord, type is {Type}.");
            GetCoordInternal(_handle, out double x, out double y, out double z, out bool is_3d);
            return new YiniCoord { X = x, Y = y, Z = z, Is3D = is_3d };
        }

        public YiniColor AsColor()
        {
            if (Type != YiniType.Color) throw new InvalidCastException($"Cannot get value as Color, type is {Type}.");
            GetColorInternal(_handle, out byte r, out byte g, out byte b);
            return new YiniColor { R = r, G = g, B = b };
        }

        public string AsPath()
        {
            if (Type != YiniType.Path) throw new InvalidCastException($"Cannot get value as Path, type is {Type}.");
            int requiredSize = GetPathInternal(_handle, null, 0);
            if (requiredSize <= 0) return string.Empty;
            var sb = new StringBuilder(requiredSize);
            GetPathInternal(_handle, sb, sb.Capacity);
            return sb.ToString();
        }
    }
}