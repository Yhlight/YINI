using Microsoft.Extensions.Configuration;
using System;
using System.IO;

namespace Yini.Extensions.Configuration
{
    /// <summary>
    /// Represents a YINI file as an <see cref="IConfigurationSource"/>.
    /// </summary>
    public class YiniConfigurationSource : FileConfigurationSource
    {
        /// <summary>
        /// Builds the <see cref="IConfigurationProvider"/> for this source.
        /// </summary>
        /// <param name="builder">The <see cref="IConfigurationBuilder"/>.</param>
        /// <returns>A <see cref="YiniConfigurationProvider"/>.</returns>
        public override IConfigurationProvider Build(IConfigurationBuilder builder)
        {
            EnsureDefaults(builder);
            return new YiniConfigurationProvider(this);
        }
    }

    /// <summary>
    /// A YINI file configuration provider for .NET.
    /// </summary>
    public class YiniConfigurationProvider : FileConfigurationProvider
    {
        /// <summary>
        /// Initializes a new instance with the specified source.
        /// </summary>
        /// <param name="source">The source settings.</param>
        public YiniConfigurationProvider(YiniConfigurationSource source) : base(source) { }

        /// <summary>
        /// Loads the YINI data from a stream.
        /// </summary>
        /// <param name="stream">The stream to read.</param>
        public override void Load(Stream stream)
        {
            Data = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);

            using var manager = new YiniManager();
            using var reader = new StreamReader(stream);
            manager.LoadFromString(reader.ReadToEnd());

            var sectionNames = manager.GetResolvedSectionNames();
            foreach (var sectionName in sectionNames)
            {
                var sectionData = manager.GetSectionAsDictionary(sectionName);
                if (sectionData == null) continue;

                foreach (var kvp in sectionData)
                {
                    string key = ConfigurationPath.Combine(sectionName, kvp.Key);
                    Data[key] = kvp.Value;
                }
            }
        }
    }

    /// <summary>
    /// Extension methods for adding <see cref="YiniConfigurationProvider"/>.
    /// </summary>
    public static class YiniConfigurationExtensions
    {
        /// <summary>
        /// Adds the YINI configuration provider at <paramref name="path"/> to <paramref name="builder"/>.
        /// </summary>
        /// <param name="builder">The <see cref="IConfigurationBuilder"/> to add to.</param>
        /// <param name="path">Path relative to the base path stored in
        /// <see cref="IConfigurationBuilder.Properties"/> of <paramref name="builder"/>.</param>
        /// <returns>The <see cref="IConfigurationBuilder"/>.</returns>
        public static IConfigurationBuilder AddYiniFile(this IConfigurationBuilder builder, string path)
        {
            return AddYiniFile(builder, provider: null, path: path, optional: false, reloadOnChange: false);
        }

        /// <summary>
        /// Adds the YINI configuration provider at <paramref name="path"/> to <paramref name="builder"/>.
        /// </summary>
        /// <param name="builder">The <see cref="IConfigurationBuilder"/> to add to.</param>
        /// <param name="path">Path relative to the base path stored in
        /// <see cref="IConfigurationBuilder.Properties"/> of <paramref name="builder"/>.</param>
        /// <param name="optional">Whether the file is optional.</param>
        /// <returns>The <see cref="IConfigurationBuilder"/>.</returns>
        public static IConfigurationBuilder AddYiniFile(this IConfigurationBuilder builder, string path, bool optional)
        {
            return AddYiniFile(builder, provider: null, path: path, optional: optional, reloadOnChange: false);
        }

        /// <summary>
        /// Adds a YINI configuration source to <paramref name="builder"/>.
        /// </summary>
        /// <param name="builder">The <see cref="IConfigurationBuilder"/> to add to.</param>
        /// <param name="path">Path relative to the base path stored in
        /// <see cref="IConfigurationBuilder.Properties"/> of <paramref name="builder"/>.</param>
        /// <param name="optional">Whether the file is optional.</param>
        /// <param name="reloadOnChange">Whether the configuration should be reloaded if the file changes.</param>
        /// <returns>The <see cref="IConfigurationBuilder"/>.</returns>
        public static IConfigurationBuilder AddYiniFile(this IConfigurationBuilder builder, string path, bool optional, bool reloadOnChange)
        {
            return AddYiniFile(builder, provider: null, path: path, optional: optional, reloadOnChange: reloadOnChange);
        }

        /// <summary>
        /// Adds a YINI configuration source to <paramref name="builder"/>.
        /// </summary>
        /// <param name="builder">The <see cref="IConfigurationBuilder"/> to add to.</param>
        /// <param name="provider">The <see cref="IFileProvider"/> to use to access the file.</param>
        /// <param name="path">Path relative to the base path stored in
        /// <see cref="IConfigurationBuilder.Properties"/> of <paramref name="builder"/>.</param>
        /// <param name="optional">Whether the file is optional.</param>
        /// <param name="reloadOnChange">Whether the configuration should be reloaded if the file changes.</param>
        /// <returns>The <see cref="IConfigurationBuilder"/>.</returns>
        public static IConfigurationBuilder AddYiniFile(this IConfigurationBuilder builder, Microsoft.Extensions.FileProviders.IFileProvider? provider, string path, bool optional, bool reloadOnChange)
        {
            if (builder == null)
            {
                throw new ArgumentNullException(nameof(builder));
            }
            if (string.IsNullOrEmpty(path))
            {
                throw new ArgumentException("File path must be a non-empty string.", nameof(path));
            }

            var source = new YiniConfigurationSource
            {
                FileProvider = provider,
                Path = path,
                Optional = optional,
                ReloadOnChange = reloadOnChange
            };
            builder.Add(source);
            return builder;
        }
    }
}