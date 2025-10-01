using System;
using System.Runtime.InteropServices;
using System.Text;

namespace YINI
{
    public class YiniSection
    {
        private const string LibName = "yini";
        private readonly IntPtr _handle;

        [DllImport(LibName, EntryPoint = "yini_section_get_name")]
        private static extern int GetNameInternal(IntPtr handle, StringBuilder? buffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_section_get_value_by_key")]
        private static extern IntPtr GetValueByKeyInternal(IntPtr handle, string key);

        internal YiniSection(IntPtr handle)
        {
            if (handle == IntPtr.Zero) throw new ArgumentNullException(nameof(handle));
            _handle = handle;
        }

        public string Name
        {
            get
            {
                int requiredSize = GetNameInternal(_handle, null, 0);
                if (requiredSize <= 0) return string.Empty;

                var sb = new StringBuilder(requiredSize);
                GetNameInternal(_handle, sb, sb.Capacity);
                return sb.ToString();
            }
        }

        public YiniValue? GetValue(string key)
        {
            IntPtr valueHandle = GetValueByKeyInternal(_handle, key);
            if (valueHandle == IntPtr.Zero)
            {
                return null;
            }
            // The returned YiniValue handle is not owned by this C# wrapper
            return new YiniValue(valueHandle, isManaged: false);
        }
    }
}