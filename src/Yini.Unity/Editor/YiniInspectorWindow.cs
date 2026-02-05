#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;
using Yini;
using Yini.Model;
using System.Collections.Generic;

namespace Yini.Unity.Editor
{
    public class YiniInspectorWindow : EditorWindow
    {
        [MenuItem("Window/Yini/Runtime Inspector")]
        public static void ShowWindow()
        {
            GetWindow<YiniInspectorWindow>("Yini Inspector");
        }

        private Vector2 _scrollPos;
        private bool _showSections = true;
        private Dictionary<string, bool> _foldouts = new Dictionary<string, bool>();

        private void OnGUI()
        {
            if (!Application.isPlaying)
            {
                EditorGUILayout.HelpBox("Enter Play Mode to inspect Runtime Configuration.", MessageType.Info);
                return;
            }

            var manager = YiniManager.Instance;
            if (manager == null)
            {
                EditorGUILayout.HelpBox("No YiniManager instance found.", MessageType.Warning);
                return;
            }

            var doc = manager.Document;
            if (doc == null)
            {
                EditorGUILayout.HelpBox("YiniManager has no loaded document.", MessageType.Warning);
                return;
            }

            _scrollPos = EditorGUILayout.BeginScrollView(_scrollPos);

            EditorGUILayout.LabelField("Runtime Configuration", EditorStyles.boldLabel);

            _showSections = EditorGUILayout.Foldout(_showSections, $"Sections ({doc.Sections.Count})");
            if (_showSections)
            {
                EditorGUI.indentLevel++;
                foreach (var section in doc.Sections.Values)
                {
                    DrawSection(section);
                }
                EditorGUI.indentLevel--;
            }

            EditorGUILayout.EndScrollView();
        }

        private void DrawSection(YiniSection section)
        {
            if (!_foldouts.ContainsKey(section.Name)) _foldouts[section.Name] = false;

            _foldouts[section.Name] = EditorGUILayout.Foldout(_foldouts[section.Name], $"[{section.Name}]");
            if (_foldouts[section.Name])
            {
                EditorGUI.indentLevel++;
                foreach (var prop in section.Properties)
                {
                    EditorGUILayout.BeginHorizontal();
                    EditorGUILayout.LabelField(prop.Key, GUILayout.Width(150));
                    EditorGUILayout.LabelField(prop.Value.ToString());
                    EditorGUILayout.EndHorizontal();
                }
                EditorGUI.indentLevel--;
            }
        }
    }
}
#endif
