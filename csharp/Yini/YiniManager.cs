using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Yini
{
    public class YiniManager : IDisposable
    {
        private const string LibName = "Yini"; // Assumes libYini.so or Yini.dll
        private IntPtr _managerPtr;
        private bool _disposed = false;

        #region PInvoke Signatures
        [DllImport(LibName, EntryPoint = "yini_manager_create")]
        private static extern IntPtr YiniManager_Create();

        [DllImport(LibName, EntryPoint = "yini_manager_destroy")]
        private static extern void YiniManager_Destroy(IntPtr manager);

        [DllImport(LibName, EntryPoint = "yini_manager_load")]
        private static extern bool YiniManager_Load(IntPtr manager, string filepath);

        [DllImport(LibName, EntryPoint = "yini_manager_save_changes")]
        private static extern void YiniManager_SaveChanges(IntPtr manager);

        [DllImport(LibName, EntryPoint = "yini_manager_get_double")]
        private static extern bool YiniManager_GetDouble(IntPtr manager, string section, string key, out double outValue);

        [DllImport(LibName, EntryPoint = "yini_manager_get_string")]
        private static extern bool YiniManager_GetString(IntPtr manager, string section, string key, StringBuilder outBuffer, int bufferSize);

        [DllImport(LibName, EntryPoint = "yini_manager_get_bool")]
        private static extern bool YiniManager_GetBool(IntPtr manager, string section, string key, out bool outValue);

        [DllImport(LibName, EntryPoint = "yini_manager_set_double")]
        private static extern void YiniManager_SetDouble(IntPtr manager, string section, string key, double value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_string")]
        private static extern void YiniManager_SetString(IntPtr manager, string section, string key, string value);

        [DllImport(LibName, EntryPoint = "yini_manager_set_bool")]
        private static extern void YiniManager_SetBool(IntPtr manager, string section, string key, bool value);
        #endregion

        public YiniManager()
        {
            _managerPtr = YiniManager_Create();
            if (_managerPtr == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to create YiniManager instance.");
            }
        }

        public bool Load(string filepath)
        {
            return YiniManager_Load(_managerPtr, filepath);
        }

        public void SaveChanges()
        {
            YiniManager_SaveChanges(_managerPtr);
        }

        public double GetDouble(string section, string key, double defaultValue = 0.0)
        {
            if (YiniManager_GetDouble(_managerPtr, section, key, out double value))
            {
                return value;
            }
            return defaultValue;
        }

        public string GetString(string section, string key, string defaultValue = "")
        {
            StringBuilder buffer = new StringBuilder(1024);
            if (YiniManager_GetString(_managerPtr, section, key, buffer, buffer.Capacity))
            {
                return buffer.ToString();
            }
            return defaultValue;
        }

        public bool GetBool(string section, string key, bool defaultValue = false)
        {
            if (YiniManager_GetBool(_managerPtr, section, key, out bool value))
            {
                return value;
            }
            return defaultValue;
        }

        public void SetDouble(string section, string key, double value)
        {
            YiniManager_SetDouble(_managerPtr, section, key, value);
        }

        public void SetString(string section, string key, string value)
        {
            YiniManager_SetString(_managerPtr, section, key, value);
        }

        public void SetBool(string section, string key, bool value)
        {
            YiniManager_SetBool(_managerPtr, section, key, value);
        }

        #region IDisposable Implementation
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_managerPtr != IntPtr.Zero)
                {
                    YiniManager_Destroy(_managerPtr);
                    _managerPtr = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~YiniManager()
        {
            Dispose(false);
        }
        #endregion
    }
}