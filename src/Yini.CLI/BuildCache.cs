using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace Yini.CLI
{
    public class BuildCache
    {
        public Dictionary<string, string> FileHashes { get; set; } = new Dictionary<string, string>();

        public static BuildCache Load(string path)
        {
            if (File.Exists(path))
            {
                try
                {
                    string json = File.ReadAllText(path);
                    return JsonSerializer.Deserialize<BuildCache>(json) ?? new BuildCache();
                }
                catch { }
            }
            return new BuildCache();
        }

        public void Save(string path)
        {
            var options = new JsonSerializerOptions { WriteIndented = true };
            string json = JsonSerializer.Serialize(this, options);
            File.WriteAllText(path, json);
        }

        public bool IsUpToDate(string filePath, string content)
        {
            string currentHash = ComputeHash(content);
            if (FileHashes.TryGetValue(filePath, out var oldHash))
            {
                return oldHash == currentHash;
            }
            return false;
        }

        public void Update(string filePath, string content)
        {
            FileHashes[filePath] = ComputeHash(content);
        }

        private static string ComputeHash(string content)
        {
            using (var sha256 = SHA256.Create())
            {
                byte[] bytes = Encoding.UTF8.GetBytes(content);
                byte[] hash = sha256.ComputeHash(bytes);
                return Convert.ToBase64String(hash);
            }
        }
    }
}
