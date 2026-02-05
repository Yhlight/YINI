#if UNITY_EDITOR
using System.IO;
using UnityEngine;

namespace Yini.Unity
{
    public class YiniHotReloader : MonoBehaviour
    {
        public YiniAsset AssetToWatch;

        private FileSystemWatcher _watcher;
        private string _watchedPath;

        private void Start()
        {
            if (AssetToWatch != null)
            {
                // Assuming Source in AssetToWatch is just a string, we can't watch it directly if it's not a file.
                // But usually in Unity, you watch the file path.
                // For this example, let's assume we want to watch a specific file path provided in a component,
                // or we try to find the asset path.
                // Resolving asset path in runtime is tricky without UnityEditor namespace.
                // But this script is #if UNITY_EDITOR, so we are good.

                string assetPath = UnityEditor.AssetDatabase.GetAssetPath(AssetToWatch);
                if (!string.IsNullOrEmpty(assetPath))
                {
                    WatchFile(Path.GetFullPath(assetPath));
                }
            }
        }

        public void WatchFile(string fullPath)
        {
            _watchedPath = fullPath;
            string dir = Path.GetDirectoryName(fullPath);
            string file = Path.GetFileName(fullPath);

            _watcher = new FileSystemWatcher(dir, file);
            _watcher.NotifyFilter = NotifyFilters.LastWrite;
            _watcher.Changed += OnFileChanged;
            _watcher.EnableRaisingEvents = true;
            Debug.Log($"[Yini] Watching {file} for changes...");
        }

        private void OnFileChanged(object sender, FileSystemEventArgs e)
        {
            // Unity callbacks must run on main thread
            UnityEditor.EditorApplication.delayCall += () =>
            {
                Debug.Log($"[Yini] Config changed: {e.Name}. Reloading...");

                // Read text
                string text = File.ReadAllText(_watchedPath);

                // Update Asset
                AssetToWatch.Source = text;
                UnityEditor.EditorUtility.SetDirty(AssetToWatch);

                // Reload Runtime
                if (YiniManager.Instance != null)
                {
                    YiniManager.Instance.LoadConfig(text);
                }
            };
        }

        private void OnDestroy()
        {
            if (_watcher != null)
            {
                _watcher.Dispose();
                _watcher = null;
            }
        }
    }
}
#endif
