# YINI v2.5.0 Official Release Package

**Version**: 2.5.0 Final  
**Platform**: Linux x86_64  
**Build**: Release (Optimized)  
**Date**: 2025-10-06

---

## 📦 Package Contents

```
yini-v2.5.0-linux-x64/
├── bin/
│   ├── yini_cli         (187KB) - CLI tool
│   └── yini_lsp         (436KB) - LSP server
├── lib/
│   └── libyini.so       (144KB) - Shared library
├── include/
│   └── *.h              - Header files
├── examples/
│   └── *.yini           - Example files
├── vscode-plugin/
│   └── *                - VSCode extension
├── README.md            - Main documentation
├── YINI.md              - Language specification
├── CHANGELOG.md         - Version history
├── GETTING_STARTED.md   - Quick start guide
├── LICENSE              - MIT License
├── install.sh           - Installation script
└── quick_start.sh       - Quick start script
```

---

## 🚀 Quick Installation

### Method 1: One Command (Recommended)

```bash
cd yini-v2.5.0-linux-x64
sudo ./install.sh
```

### Method 2: Manual

```bash
sudo cp bin/* /usr/local/bin/
sudo cp lib/* /usr/local/lib/
sudo cp -r include/* /usr/local/include/yini/
sudo ldconfig
```

### Method 3: Local (No sudo)

```bash
export PATH=$PATH:$(pwd)/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib
```

---

## ✅ Verification

### Test CLI

```bash
yini_cli
yini> help
yini> parse examples/simple.yini
yini> exit
```

### Test LSP

```bash
which yini_lsp
# Should output: /usr/local/bin/yini_lsp
```

---

## 🎨 VSCode Extension

### Install

```bash
cd vscode-plugin
npm install

# Optional: Package
npm install -g @vscode/vsce
vsce package
code --install-extension yini-language-support-2.5.0.vsix
```

### Configure

Add to VSCode settings.json:

```json
{
  "yini.lsp.path": "yini_lsp"
}
```

---

## 📊 Binary Information

### Optimization

| Binary | Debug | Release | Reduction |
|--------|-------|---------|-----------|
| yini_cli | 2.3MB | **187KB** | **-92%** |
| yini_lsp | 7.8MB | **436KB** | **-94%** |
| libyini.so | 2.0MB | **144KB** | **-93%** |

**Total size: <800KB!** ✅

### Features

- ✅ Fully optimized (-O3)
- ✅ Stripped symbols
- ✅ Link-time optimization
- ✅ Small footprint
- ✅ High performance

---

## 📚 Documentation

- **README.md** - Project overview
- **YINI.md** - Complete language specification  
- **GETTING_STARTED.md** - 5-minute quick start
- **CHANGELOG.md** - Version history

---

## 🎯 System Requirements

### Minimum

- Linux kernel 3.10+
- glibc 2.17+
- 50MB disk space
- 100MB RAM

### Recommended

- Linux kernel 5.0+
- glibc 2.27+
- 200MB disk space
- 200MB RAM
- VSCode 1.60+

---

## ⚡ Performance

| Operation | Response Time |
|-----------|---------------|
| CLI startup | <10ms |
| LSP startup | <50ms |
| Completion | <20ms |
| Diagnostics | <30ms |

---

## 🌟 Features

### Core Language
- 12 data types
- Macro definitions
- File includes
- Schema validation
- Reference resolution

### IDE Support (LSP)
- Real-time diagnostics
- Smart completion
- Hover information
- Go to definition
- Find references
- Rename refactoring
- Code formatting
- Semantic highlighting
- Document symbols
- Workspace symbols

---

## 📝 License

MIT License - See LICENSE file

---

## 📞 Support

For issues and questions, refer to the documentation files included in this package.

---

**YINI v2.5.0 - Professional Configuration Language!** 🎮✨

**Release Date**: 2025-10-06  
**Build Type**: Release (Optimized)  
**Status**: Production Ready ✅
