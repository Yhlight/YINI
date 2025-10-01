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
        List,
        Set,
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
        private static extern int GetStringInternal(IntPtr handle, StringBuilder? buffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_value_get_int")]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool GetIntInternal(IntPtr handle, out int outValue);

        [DllImport(LibName, EntryPoint = "yini_value_get_double")]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool GetDoubleInternal(IntPtr handle, out double outValue);

        [DllImport(LibName, EntryPoint = "yini_value_get_bool")]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool GetBoolInternal(IntPtr handle, [MarshalAs(UnmanagedType.I1)] out bool outValue);

        [DllImport(LibName, EntryPoint = "yini_array_get_size")]
        private static extern int GetArraySizeInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_array_get_value_by_index")]
        private static extern IntPtr GetArrayValueByIndexInternal(IntPtr handle, int index);

        [DllImport(LibName, EntryPoint = "yini_list_get_size")]
        private static extern int GetListSizeInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_list_get_value_by_index")]
        private static extern IntPtr GetListValueByIndexInternal(IntPtr handle, int index);

        [DllImport(LibName, EntryPoint = "yini_set_get_size")]
        private static extern int GetSetSizeInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_set_get_value_by_index")]
        private static extern IntPtr GetSetValueByIndexInternal(IntPtr handle, int index);

        [DllImport(LibName, EntryPoint = "yini_map_get_size")]
        private static extern int GetMapSizeInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_map_get_key_by_index")]
        private static extern int GetMapKeyByIndexInternal(IntPtr handle, int index, StringBuilder? buffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_map_get_value_by_key")]
        private static extern IntPtr GetMapValueByKeyInternal(IntPtr handle, [MarshalAs(UnmanagedType.LPStr)] string key);

        [DllImport(LibName, EntryPoint = "yini_dyna_get_value")]
        private static extern IntPtr GetDynaValueInternal(IntPtr handle);

        [DllImport(LibName, EntryPoint = "yini_value_get_coord")]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool GetCoordInternal(IntPtr handle, out double x, out double y, out double z, [MarshalAs(UnmanagedType.I1)] out bool is_3d);

        [DllImport(LibName, EntryPoint = "yini_value_get_color")]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool GetColorInternal(IntPtr handle, out byte r, out byte g, out byte b);

        [DllImport(LibName, EntryPoint = "yini_value_get_path")]
        private static extern int GetPathInternal(IntPtr handle, StringBuilder? buffer, int bufferSize);

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
            if (!GetIntInternal(_handle, out int value))
            {
                throw new InvalidOperationException("Failed to retrieve integer value from native library.");
            }
            return value;
        }

        public double AsDouble()
        {
            if (Type != YiniType.Double) throw new InvalidCastException($"Cannot get value as double, type is {Type}.");
            if (!GetDoubleInternal(_handle, out double value))
            {
                throw new InvalidOperationException("Failed to retrieve double value from native library.");
            }
            return value;
        }

        public bool AsBool()
        {
            if (Type != YiniType.Bool) throw new InvalidCastException($"Cannot get value as bool, type is {Type}.");
            if (!GetBoolInternal(_handle, out bool value))
            {
                throw new InvalidOperationException("Failed to retrieve boolean value from native library.");
            }
            return value;
        }

        public YiniValue[] AsArray()
        {
            if (Type != YiniType.Array) throw new InvalidCastException($"Cannot get value as array, type is {Type}.");
            int size = GetArraySizeInternal(_handle);
            var array = new YiniValue[size];
            for (int i = 0; i < size; i++)
            {
                IntPtr valueHandle = GetArrayValueByIndexInternal(_handle, i);
                if (valueHandle != IntPtr.Zero)
                {
                    array[i] = new YiniValue(valueHandle);
                }
            }
            return array;
        }

        public YiniValue[] AsList()
        {
            if (Type != YiniType.List) throw new InvalidCastException($"Cannot get value as list, type is {Type}.");
            int size = GetListSizeInternal(_handle);
            var list = new YiniValue[size];
            for (int i = 0; i < size; i++)
            {
                IntPtr valueHandle = GetListValueByIndexInternal(_handle, i);
                if (valueHandle != IntPtr.Zero)
                {
                    list[i] = new YiniValue(valueHandle);
                }
            }
            return list;
        }

        public YiniValue[] AsSet()
        {
            if (Type != YiniType.Set) throw new InvalidCastException($"Cannot get value as set, type is {Type}.");
            int size = GetSetSizeInternal(_handle);
            var set = new YiniValue[size];
            for (int i = 0; i < size; i++)
            {
                IntPtr valueHandle = GetSetValueByIndexInternal(_handle, i);
                if (valueHandle != IntPtr.Zero)
                {
                    set[i] = new YiniValue(valueHandle);
                }
            }
            return set;
        }

        public Dictionary<string, YiniValue> AsMap()
        {
            if (Type != YiniType.Map) throw new InvalidCastException($"Cannot get value as map, type is {Type}.");
            int size = GetMapSizeInternal(_handle);
            var map = new Dictionary<string, YiniValue>(size);
            for (int i = 0; i < size; i++)
            {
                int keySize = GetMapKeyByIndexInternal(_handle, i, null, 0);
                if (keySize <= 0) continue;
                var keySb = new StringBuilder(keySize);
                GetMapKeyByIndexInternal(_handle, i, keySb, keySb.Capacity);
                string key = keySb.ToString();

                IntPtr valueHandle = GetMapValueByKeyInternal(_handle, key);
                if (valueHandle != IntPtr.Zero)
                {
                    map[key] = new YiniValue(valueHandle);
                }
            }
            return map;
        }

        public YiniValue AsDyna()
        {
            if (Type != YiniType.Dyna) throw new InvalidCastException($"Cannot get value as Dyna, type is {Type}.");
            IntPtr innerHandle = GetDynaValueInternal(_handle);
            if (innerHandle == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to retrieve inner value from Dyna value.");
            }
            return new YiniValue(innerHandle);
        }

        public YiniCoord AsCoord()
        {
            if (Type != YiniType.Coord) throw new InvalidCastException($"Cannot get value as Coord, type is {Type}.");
            if (!GetCoordInternal(_handle, out double x, out double y, out double z, out bool is_3d))
            {
                throw new InvalidOperationException("Failed to retrieve Coord value from native library.");
            }
            return new YiniCoord { X = x, Y = y, Z = z, Is3D = is_3d };
        }

        public YiniColor AsColor()
        {
            if (Type != YiniType.Color) throw new InvalidCastException($"Cannot get value as Color, type is {Type}.");
            if (!GetColorInternal(_handle, out byte r, out byte g, out byte b))
            {
                throw new InvalidOperationException("Failed to retrieve Color value from native library.");
            }
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