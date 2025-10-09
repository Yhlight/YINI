using System.Collections.Generic;
using System.Linq;

namespace Yini.Core
{
    public class DynaValue<T>
    {
        private readonly YiniConfig _config;
        private readonly string _section;
        private readonly string _key;
        private T _value;

        public List<T> Backups { get; }

        public T Value
        {
            get => _value;
            set
            {
                if (!Equals(_value, value))
                {
                    // Add the old value to backups
                    if (Backups.Count >= 5)
                    {
                        Backups.RemoveAt(0);
                    }
                    Backups.Add(_value);

                    _value = value;

                    // Update the underlying native config
                    if (_value is string s) _config.SetString(_section, _key, s);
                    else if (_value is int i) _config.SetInt(_section, _key, i);
                    else if (_value is double d) _config.SetDouble(_section, _key, d);
                    else if (_value is bool b) _config.SetBool(_section, _key, b);

                    _config.MarkAsDirty();
                }
            }
        }

        internal DynaValue(YiniConfig config, string section, string key, T initialValue, IEnumerable<T> backups)
        {
            _config = config;
            _section = section;
            _key = key;
            _value = initialValue;
            Backups = backups?.ToList() ?? new List<T>();
        }
    }
}