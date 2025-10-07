# YINI v2.5.0 Official Installation Guide

**For**: Official Release Testing  
**Version**: 2.5.0 Final  
**Platform**: Linux x86_64

---

## 📦 Package Information

**Filename**: `yini-v2.5.0-linux-x64.tar.gz`  
**Size**: 468KB (compressed)  
**Extracted**: 1.4MB  
**Files**: 45 files

**Checksums**:
- MD5: `af9dc3b831cf827f36a6bab82b36f179`
- SHA256: `0c98fa3982103a646dbc20768c1814f2fe268eb654c83c6b15c6955398eb3c5e`

---

## 🚀 Installation Steps

### Step 1: Extract Package

```bash
tar -xzf yini-v2.5.0-linux-x64.tar.gz
cd yini-v2.5.0-linux-x64
```

### Step 2: Verify Integrity

```bash
sha256sum -c SHA256SUMS
```

Expected output:
```
bin/yini_cli: OK
bin/yini_lsp: OK
lib/libyini.so.1.0.0: OK
```

### Step 3: Install

```bash
sudo ./install.sh
```

Or custom location:
```bash
INSTALL_PREFIX=$HOME/.local ./install.sh
```

### Step 4: Verify Installation

```bash
# Check CLI
yini_cli --version
which yini_cli

# Check LSP
which yini_lsp

# Check library
ldconfig -p | grep yini
```

---

## ✅ Testing Checklist

### Basic Tests

- [ ] Extract package successfully
- [ ] Checksums verify correctly
- [ ] Installation script runs without errors
- [ ] yini_cli executable found in PATH
- [ ] yini_lsp executable found in PATH
- [ ] libyini.so found in library path

### Functional Tests

- [ ] yini_cli starts and shows prompt
- [ ] `help` command works
- [ ] Can parse `examples/simple.yini`
- [ ] yini_lsp starts (waits for input)
- [ ] No error messages on startup

### IDE Tests (VSCode)

- [ ] Install VSCode extension
- [ ] Configure LSP path
- [ ] Open .yini file
- [ ] Syntax highlighting works
- [ ] Auto-completion works (type `@`)
- [ ] Hover information works
- [ ] F12 go to definition works
- [ ] No errors in Output panel

---

## 📋 Binary Information

### yini_cli (187KB)

- **Type**: ELF 64-bit LSB executable
- **Platform**: x86-64, Linux
- **Stripped**: Yes (symbols removed)
- **Optimized**: -O3
- **Function**: Interactive CLI tool

**Test**:
```bash
./bin/yini_cli
yini> help
yini> parse examples/simple.yini
yini> exit
```

### yini_lsp (436KB)

- **Type**: ELF 64-bit LSB executable
- **Platform**: x86-64, Linux
- **Stripped**: Yes
- **Optimized**: -O3
- **Function**: Language Server Protocol server

**Test**:
```bash
./bin/yini_lsp &
# Should wait for JSON-RPC input
# Press Ctrl+C to stop
```

### libyini.so (176KB)

- **Type**: ELF 64-bit LSB shared object
- **Platform**: x86-64, Linux
- **Version**: 1.0.0
- **Function**: YINI parser library

---

## 🔍 Verification Commands

### Quick Verification

```bash
# 1. File existence
ls -lh bin/yini_*
ls -lh lib/libyini.so*

# 2. File type
file bin/yini_cli
file bin/yini_lsp

# 3. Dependencies
ldd bin/yini_cli
ldd bin/yini_lsp

# 4. Execution
./bin/yini_cli <<EOF
help
exit
EOF
```

### Complete Verification

```bash
# Run included verification script
./verify_installation.sh
```

---

## ⚠️ Troubleshooting

### Issue: "Command not found"

**Solution**: Add to PATH or use full path
```bash
export PATH=$PATH:/usr/local/bin
```

### Issue: "Library not found"

**Solution**: Update library cache
```bash
sudo ldconfig
```

### Issue: "Permission denied"

**Solution**: Make files executable
```bash
chmod +x bin/*
```

---

## 🗑️ Uninstallation

### If installed to /usr/local:

```bash
sudo rm /usr/local/bin/yini_cli
sudo rm /usr/local/bin/yini_lsp
sudo rm /usr/local/lib/libyini.so*
sudo rm -rf /usr/local/include/yini
sudo ldconfig
```

### If installed to custom location:

```bash
rm $INSTALL_PREFIX/bin/yini_*
rm $INSTALL_PREFIX/lib/libyini.so*
rm -rf $INSTALL_PREFIX/include/yini
```

---

## 📊 Package Contents Summary

```
Total Size: 1.4MB (extracted)
Total Files: 45

Distribution:
├── Binaries: 3 files, 799KB
├── Headers: 17 files, ~50KB
├── Examples: 4 files, ~10KB
├── Documentation: 5 files, ~100KB
├── VSCode Extension: ~15 files
└── Scripts: 2 files
```

---

## ✅ Acceptance Criteria

Package is ready for deployment if:

- ✅ All binaries execute without errors
- ✅ Checksums verify correctly
- ✅ Installation completes successfully
- ✅ Basic functionality works
- ✅ LSP integrates with VSCode
- ✅ No security warnings
- ✅ Documentation is complete

---

## 📝 Notes for Official Release

### This Package Contains:

1. **Production-ready binaries** (Release build, stripped)
2. **Complete documentation** (4 core docs)
3. **Working examples** (4 .yini files)
4. **VSCode extension** (Ready to package)
5. **Installation scripts** (Automated setup)

### Recommended Actions:

1. ✅ Test on clean Linux system
2. ✅ Verify all functionality
3. ✅ Check VSCode integration
4. ✅ Review documentation
5. ✅ Test installation/uninstallation

### After Testing:

- If approved: Publish to official channels
- If issues found: Report for fixes
- Safe to delete after deployment

---

**Package prepared by**: YINI Development Team  
**Build date**: 2025-10-06  
**Quality**: Production grade ✅  
**Status**: Ready for official release 🚀
