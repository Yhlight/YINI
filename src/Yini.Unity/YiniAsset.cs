using UnityEngine;

namespace Yini.Unity
{
    [CreateAssetMenu(fileName = "New Yini Config", menuName = "Yini/Config Asset")]
    public class YiniAsset : ScriptableObject
    {
        [TextArea(10, 20)]
        public string Source;

        [HideInInspector]
        public byte[] BinaryData;
    }
}
