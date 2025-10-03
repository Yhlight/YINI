using System;

namespace Yini
{
    /// <summary>
    /// Specifies the key name to use when binding a YINI section to a C# object.
    /// If this attribute is not present, the property name (converted to lowercase) will be used by default.
    /// </summary>
    [AttributeUsage(AttributeTargets.Property, Inherited = false, AllowMultiple = false)]
    public sealed class YiniKeyAttribute : Attribute
    {
        /// <summary>
        /// The name of the key in the YINI file.
        /// </summary>
        public string Key { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="YiniKeyAttribute"/> class.
        /// </summary>
        /// <param name="key">The name of the key in the YINI file.</param>
        public YiniKeyAttribute(string key)
        {
            Key = key;
        }
    }
}