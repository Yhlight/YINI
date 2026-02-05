using System.IO;

namespace Yini
{
    public interface IFileLoader
    {
        string LoadFile(string path);
        bool Exists(string path);
    }

    public class PhysicalFileLoader : IFileLoader
    {
        private readonly string _baseDir;
        public PhysicalFileLoader(string baseDir) => _baseDir = baseDir;

        public string LoadFile(string path)
        {
            return File.ReadAllText(Path.Combine(_baseDir, path));
        }

        public bool Exists(string path)
        {
            return File.Exists(Path.Combine(_baseDir, path));
        }
    }
}
