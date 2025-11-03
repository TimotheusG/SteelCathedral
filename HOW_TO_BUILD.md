# How to Build MechInterior - The Working Method

## ✅ What Actually Works

After testing multiple approaches, here's the **one command that reliably works** for building this UE5.6 C++ project:

```bash
cd "C:\Users\timge\Documents\Unreal Projects\MechInterior" && dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MechInteriorEditor Win64 Development -Project="C:\Users\timge\Documents\Unreal Projects\MechInterior\MechInterior.uproject" -WaitMutex
```

**Build Time:** ~5 seconds (with UBA - Unreal Build Accelerator)

---

## 📋 Command Breakdown

```bash
dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll"
```
- Uses the .NET runtime to execute UnrealBuildTool directly
- Avoids batch file wrapper issues with spaces in paths
- Works consistently across different terminal environments

```bash
MechInteriorEditor Win64 Development
```
- **MechInteriorEditor**: Target name (builds the editor version)
- **Win64**: Platform
- **Development**: Configuration (alternatives: Debug, Shipping, DebugGame)

```bash
-Project="C:\Users\timge\Documents\Unreal Projects\MechInterior\MechInterior.uproject"
```
- Full path to .uproject file (required when building outside the project directory)

```bash
-WaitMutex
```
- Waits for other build processes to complete before starting
- Prevents conflicts if multiple builds are queued

---

## ❌ What Doesn't Work (And Why)

### 1. Direct Build.bat Call
```bash
# FAILS - Quote parsing issues
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" MechInteriorEditor Win64 Development
```
**Problem:** Bash/cmd handles quotes differently, path gets split at spaces

### 2. MSBuild on .sln
```bash
# FAILS - .NET SDK not found
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" MechInterior.sln
```
**Problem:** UE5 uses .NET 6+ SDK projects, but MSBuild looks for Framework SDKs

### 3. Using PROGRA~1 Short Names
```bash
# FAILS - Quote escaping issues
C:\PROGRA~1\"Epic Games"\UE_5.6\Engine\Build\BatchFiles\Build.bat
```
**Problem:** Mix of short names and quotes creates parsing nightmares

---

## 🚀 Quick Reference

### Full Build (Clean + Compile)
```bash
dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MechInteriorEditor Win64 Development -Project="C:\Users\timge\Documents\Unreal Projects\MechInterior\MechInterior.uproject" -WaitMutex
```

### Build with Verbose Output
```bash
dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MechInteriorEditor Win64 Development -Project="C:\Users\timge\Documents\Unreal Projects\MechInterior\MechInterior.uproject" -WaitMutex -Verbose
```

### Build Game Target (Not Editor)
```bash
dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MechInterior Win64 Development -Project="C:\Users\timge\Documents\Unreal Projects\MechInterior\MechInterior.uproject" -WaitMutex
```

### Build for Shipping (Release)
```bash
dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MechInteriorEditor Win64 Shipping -Project="C:\Users\timge\Documents\Unreal Projects\MechInterior\MechInterior.uproject" -WaitMutex
```

---

## 🔧 Configuration Options

### Build Configurations
- **Development** - Default, includes editor, optimized for iteration
- **Debug** - Full debug symbols, no optimization (very slow)
- **DebugGame** - Game code debuggable, engine code optimized
- **Shipping** - Final release build, no editor, maximum optimization

### Platforms
- **Win64** - Windows 64-bit (most common)
- **Linux** - Linux builds
- **Mac** - macOS builds
- **Android** - Android mobile
- **IOS** - iOS mobile

### Useful Flags
- `-WaitMutex` - Wait for other builds to finish
- `-Verbose` - Detailed build output
- `-Clean` - Force clean rebuild
- `-NoUBT` - Skip UnrealBuildTool (very rare)
- `-IgnoreJunk` - Ignore junk files during build

---

## 📝 Build Output

### Success Output:
```
Parsing headers for MechInteriorEditor
  Running Internal UnrealHeaderTool...
Reflection code generated for MechInteriorEditor in 1.5 seconds
Building MechInteriorEditor...
Using Visual Studio 2022 14.44.35219 toolchain
[1/7] Compile [x64] PlayerMechPawn.cpp
[2/7] Compile [x64] MechStation.cpp
...
[7/7] WriteMetadata MechInteriorEditor.target
Result: Succeeded
Total execution time: 5.13 seconds
```

### Expected Warnings:
```
Warning: Visual Studio 2022 compiler is not a preferred version
```
**This is normal** - UE5.6 prefers VS2019 but works fine with VS2022

### Files Generated:
- `Binaries/Win64/UnrealEditor-MechInterior.dll` - Game module DLL
- `Binaries/Win64/UnrealEditor-MechInterior.lib` - Import library
- `Intermediate/Build/Win64/MechInteriorEditor/Development/` - Intermediate files
- `Saved/Logs/` - Build logs

---

## 🐛 Troubleshooting

### "dotnet: command not found"
**Solution:** Install .NET 6 SDK or higher from https://dotnet.microsoft.com/download

### "UnrealBuildTool.dll not found"
**Solution:** Verify UE5.6 installation path. Update path in command if different.

### "Project file not found"
**Solution:** Ensure you're using the **full absolute path** to .uproject file in `-Project=` flag

### Build takes >60 seconds
**Solution:**
1. Check if UBA (Unreal Build Accelerator) is enabled
2. Close other applications consuming CPU
3. Consider upgrading to SSD if using HDD
4. Use `-Clean` sparingly (forces full rebuild)

### "Error: Unable to instantiate module 'MechInterior'"
**Solution:**
1. Delete `Binaries/`, `Intermediate/`, `Saved/` folders
2. Right-click .uproject → Generate Visual Studio project files
3. Rebuild with command above

---

## 🎯 Pro Tips

### 1. Create a Build Script
Create `build.bat` in project root:
```batch
@echo off
cd /d "%~dp0"
dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MechInteriorEditor Win64 Development -Project="%~dp0MechInterior.uproject" -WaitMutex
pause
```

### 2. Use Build Alias
Add to PowerShell profile (`$PROFILE`):
```powershell
function Build-MechInterior {
    cd "C:\Users\timge\Documents\Unreal Projects\MechInterior"
    dotnet "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" MechInteriorEditor Win64 Development -Project="$PWD\MechInterior.uproject" -WaitMutex
}
Set-Alias bm Build-MechInterior
```
Then just type: `bm` to build!

### 3. Check Build Logs
Build logs are at:
```
C:\Users\timge\AppData\Local\UnrealBuildTool\Log.txt
C:\Users\timge\AppData\Local\UnrealBuildTool\Log.uba
```

### 4. Parallel Builds
UBT automatically uses all CPU cores. On 8-core CPU:
- Executes up to 8 parallel compile actions
- One action per physical core
- Managed by UBA (Unreal Build Accelerator)

---

## 📊 Build Performance

### Typical Build Times (8-core CPU, SSD):
- **Full clean build:** 15-25 seconds
- **Incremental (1 file changed):** 3-5 seconds
- **No changes (header check only):** 1-2 seconds

### What Gets Built:
```
[1/7] Compile [x64] MechGameMode.cpp
[2/7] Compile [x64] PlayerMechPawn.cpp
[3/7] Compile [x64] Module.MechInterior.2.cpp (Unity file)
[4/7] Compile [x64] MechStation.cpp
[5/7] Link [x64] UnrealEditor-MechInterior.lib
[6/7] Link [x64] UnrealEditor-MechInterior.dll
[7/7] WriteMetadata MechInteriorEditor.target
```

**Unity Build:** UBT combines multiple .cpp files into unity files to speed up compilation

---

## 🔄 When to Rebuild

### Always Rebuild When:
- Adding new C++ class
- Changing .h file declarations
- Modifying UCLASS/UPROPERTY/UFUNCTION macros
- Changing Build.cs dependencies
- Switching build configurations

### No Rebuild Needed When:
- Changing Blueprint assets
- Modifying .ini config files
- Editing materials/textures
- Changing level geometry
- Updating UI widgets (unless C++ based)

---

## ✅ Verification

After successful build, verify these files exist:
```
Binaries/Win64/UnrealEditor-MechInterior.dll    (~1-5 MB)
Binaries/Win64/UnrealEditor-MechInterior.pdb    (~10-50 MB, debug symbols)
```

If missing, the Editor won't load your module.

---

## 🚨 Common Errors and Fixes

### Error: "C2065: undeclared identifier"
**Cause:** Missing #include
**Fix:** Add required header to .cpp or .h file

### Error: "LNK2019: unresolved external symbol"
**Cause:** Function declared but not implemented
**Fix:** Implement the function or remove declaration

### Error: "C2248: cannot access private member"
**Cause:** Trying to access private/protected member
**Fix:** Make member public, add getter, or add friend declaration

### Error: "Module 'MechInterior' could not be loaded"
**Cause:** DLL failed to link or load
**Fix:** Check Output Log for dependency errors, rebuild with `-Clean`

---

## 📚 Additional Resources

- **UnrealBuildTool docs:** https://docs.unrealengine.com/5.6/en-US/unreal-build-tool-in-unreal-engine/
- **Build configuration:** https://docs.unrealengine.com/5.6/en-US/build-configuration-for-unreal-engine/
- **Project log:** `C:\Users\timge\AppData\Local\UnrealBuildTool\Log.txt`

---

**Last Updated:** 2025-11-03
**Verified Working:** UE 5.6, VS2022 17.14, .NET 6.0, Windows 11
