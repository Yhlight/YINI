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
        Map
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
    }
}