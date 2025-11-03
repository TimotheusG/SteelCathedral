# Enabling C++ in MechInterior Project

## Why C++?
- **Better Performance:** Critical for movement, networking, and complex systems
- **More Control:** Direct access to engine features
- **Easier Debugging:** Better tools and profiling
- **Networking:** Much easier to implement multiplayer authority
- **Component Systems:** More efficient than Blueprint-only

---

## Method 1: Add C++ Class Through Editor (Recommended)

### Step 1: Add First C++ Class
1. In Unreal Editor, go to **Tools → New C++ Class**
2. Select **Actor Component** as parent class
3. Name it: `MechMovementComponent`
4. Click **Create Class**

### Step 2: Wait for Setup
- Unreal will:
  - Create `Source` folder with C++ files
  - Generate Visual Studio solution (`.sln` file)
  - Compile the new class
  - Open Visual Studio (if installed)

### Step 3: Verify Setup
Check that these files now exist:
- `Source/MechInterior/MechInterior.Build.cs`
- `Source/MechInterior/MechMovementComponent.h`
- `Source/MechInterior/MechMovementComponent.cpp`
- `MechInterior.sln` (in project root)

---

## Method 2: Manual Setup (If Method 1 Fails)

### Step 1: Create Source Folder Structure
Create these folders and files manually:

```
MechInterior/
├── Source/
│   └── MechInterior/
│       ├── MechInterior.Build.cs
│       ├── MechInterior.cpp
│       ├── MechInterior.h
│       └── MechInteriorGameModeBase.cpp
│           MechInteriorGameModeBase.h
```

### Step 2: Create Build Configuration File
**File:** `Source/MechInterior/MechInterior.Build.cs`

See the file I'll create for you below.

### Step 3: Generate Project Files
1. **Close Unreal Editor**
2. Right-click `MechInterior.uproject` in Windows Explorer
3. Select **"Generate Visual Studio project files"**
4. Wait for completion
5. Open `MechInterior.sln` in Visual Studio
6. Build the solution (Build → Build Solution or Ctrl+Shift+B)
7. Reopen project in Unreal Editor

---

## Verifying C++ is Enabled

### Check 1: Visual Studio Solution Exists
- `MechInterior.sln` should exist in project root
- Can open in Visual Studio 2022

### Check 2: Source Code Visible in Editor
1. In Unreal Editor
2. **Content Browser → C++ Classes folder** should appear
3. Should show "MechInterior" folder

### Check 3: Can Create C++ Classes
1. **Tools → New C++ Class** should work
2. No errors when creating new class

---

## What Gets Created

### Project Structure After C++ Enabled
```
MechInterior/
├── MechInterior.uproject          (Updated with modules)
├── MechInterior.sln               (Visual Studio solution)
├── Source/                        (C++ source code)
│   ├── MechInterior/
│   │   ├── MechInterior.Build.cs  (Build configuration)
│   │   ├── MechInterior.cpp       (Module implementation)
│   │   ├── MechInterior.h         (Module header)
│   │   └── [Your C++ classes]
│   └── MechInterior.Target.cs     (Build target)
│       MechInteriorEditor.Target.cs
├── Intermediate/                  (Compiled intermediate files)
├── Binaries/                      (Compiled DLLs)
└── Content/                       (Existing blueprints, assets)
```

---

## Common Issues & Solutions

### Issue: "Generate Visual Studio Files" Not Available
**Solution:**
- Install Visual Studio 2022 with "Game Development with C++" workload
- Verify `UnrealVersionSelector` is installed (comes with Epic Games Launcher)

### Issue: Build Fails - "Cannot find module"
**Solution:**
- Check `MechInterior.Build.cs` syntax
- Verify module name matches project name exactly
- Delete `Intermediate` and `Binaries` folders, regenerate

### Issue: "Hot Reload Failed" When Compiling
**Solution:**
- Close Unreal Editor completely
- Compile in Visual Studio
- Reopen Unreal Editor

### Issue: C++ Classes Not Showing in Content Browser
**Solution:**
- Refresh Content Browser (right-click → Refresh)
- Check **View Options → Show C++ Classes** is enabled
- Restart Unreal Editor

---

## Best Practices After Enabling C++

### 1. Development Workflow
- Make C++ changes in Visual Studio
- Compile in Visual Studio (not in Unreal)
- Use Hot Reload for small changes only
- For major changes: close Editor, compile, reopen

### 2. Project Organization
- Keep core systems in C++ (movement, networking, damage)
- Keep gameplay logic in Blueprints (easier iteration)
- Expose C++ functions to Blueprints with `UFUNCTION(BlueprintCallable)`
- Expose C++ variables with `UPROPERTY(BlueprintReadWrite)`

### 3. Version Control
Add to `.gitignore`:
```
Binaries/
Intermediate/
Saved/
*.sln
*.suo
*.sdf
*.opensdf
```

Keep in version control:
```
Source/
Content/
Config/
MechInterior.uproject
```

---

## Next Steps After C++ is Enabled

1. **Create Core Component Classes:**
   - `MechMovementComponent` (C++)
   - `WeaponSystemsComponent` (C++)
   - `ReactorSystemComponent` (C++)
   - `DamageManagementComponent` (C++)

2. **Create Core Actor Classes:**
   - `APlayerMechPawn` (C++)
   - `APilotStationActor` (C++)
   - `AInteriorEnvironmentActor` (C++)

3. **Blueprint Integration:**
   - Create Blueprint child classes (e.g., `BP_PlayerMechPawn` extends `APlayerMechPawn`)
   - Set up properties and assets in Blueprints
   - Keep core logic in C++

---

## Testing C++ Setup

### Simple Test Class
After C++ is enabled, create a test class:

1. **Tools → New C++ Class → Actor**
2. Name: `TestActor`
3. After creation, add to `TestActor.h`:
```cpp
UFUNCTION(BlueprintCallable, Category = "Test")
void PrintHello();
```

4. In `TestActor.cpp`:
```cpp
void ATestActor::PrintHello()
{
    UE_LOG(LogTemp, Warning, TEXT("C++ is working!"));
}
```

5. Compile
6. In Blueprint, call `PrintHello` function
7. Check Output Log - should see "C++ is working!"

If this works, C++ is fully enabled and functional!

---

## Recommended Visual Studio Extensions

- **Visual Assist** - Better IntelliSense for Unreal
- **UnrealVS Extension** - Debugging tools
- **ReSharper C++** - Code analysis

---

**Ready to enable C++? Let me know if you encounter any issues!**
