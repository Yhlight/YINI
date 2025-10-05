#nullable enable
using System;

namespace Yini
{
    /// <summary>
    /// Represents a 32-bit RGBA color.
    /// </summary>
    public struct Color : IEquatable<Color>
    {
        /// <summary> The red component of the color. </summary>
        public byte R { get; set; }
        /// <summary> The green component of the color. </summary>
        public byte G { get; set; }
        /// <summary> The blue component of the color. </summary>
        public byte B { get; set; }
        /// <summary> The alpha component of the color. </summary>
        public byte A { get; set; }

        /// <summary>
        /// Initializes a new instance of the <see cref="Color"/> struct.
        /// </summary>
        /// <param name="r">The red component.</param>
        /// <param name="g">The green component.</param>
        /// <param name="b">The blue component.</param>
        /// <param name="a">The alpha component (defaults to 255).</param>
        public Color(byte r, byte g, byte b, byte a = 255)
        {
            R = r;
            G = g;
            B = b;
            A = a;
        }

        /// <summary>
        /// Determines whether the specified object is equal to the current object.
        /// </summary>
        public override bool Equals(object? obj) => obj is Color color && Equals(color);

        /// <summary>
        /// Determines whether the specified color is equal to the current color.
        /// </summary>
        public bool Equals(Color other) => R == other.R && G == other.G && B == other.B && A == other.A;

        /// <summary>
        /// Serves as the default hash function.
        /// </summary>
        public override int GetHashCode() => HashCode.Combine(R, G, B, A);

        /// <summary>
        /// Compares two <see cref="Color"/> objects for equality.
        /// </summary>
        public static bool operator ==(Color left, Color right) => left.Equals(right);

        /// <summary>
        /// Compares two <see cref="Color"/> objects for inequality.
        /// </summary>
        public static bool operator !=(Color left, Color right) => !(left == right);
    }
}
