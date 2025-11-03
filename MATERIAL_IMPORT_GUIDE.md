# Material Import Guide - Steel Cathedral

## What Was Downloaded

I've successfully downloaded **3 complete PBR material sets** from Poly Haven (CC0 license - completely free to use):

### 1. **Rusty Metal 02** - Perfect for Mech Exterior
- `rusty_metal_02_diff_2k.jpg` - Diffuse/Albedo (1.9 MB)
- `rusty_metal_02_nor_gl_2k.jpg` - Normal Map (1.3 MB)
- `rusty_metal_02_rough_2k.jpg` - Roughness Map (402 KB)
- **Best for:** Torso, legs, arms - gives that worn, battle-scarred look

### 2. **Metal Plate** - Clean Industrial Look
- `metal_plate_diff_2k.jpg` - Diffuse/Albedo (2.6 MB)
- `metal_plate_nor_gl_2k.jpg` - Normal Map (2.5 MB)
- `metal_plate_rough_2k.jpg` - Roughness Map (3.0 MB)
- **Best for:** Interior walls, clean structural elements, head/cockpit

### 3. **Concrete Floor** - Interior Surfaces
- `concrete_floor_diff_2k.jpg` - Diffuse/Albedo (4.1 MB)
- `concrete_floor_nor_gl_2k.jpg` - Normal Map (3.9 MB)
- `concrete_floor_rough_2k.jpg` - Roughness Map (2.3 MB)
- **Best for:** Cockpit floor, reactor room floor, corridors

**Total:** 9 texture files, ~22 MB
**Location:** `Content/Materials/Downloaded/PolyHaven/`

---

## How to Import into Unreal Engine 5

### Step 1: Import Textures

1. **Open Unreal Editor**
2. **Navigate to Content Browser**
3. **Create folder structure:**
   ```
   Content/
   └── Materials/
       └── PolyHaven/
           ├── RustyMetal02/
           ├── MetalPlate/
           └── ConcreteFloor/
   ```

4. **Import textures:**
   - Right-click in Content Browser → Import
   - Navigate to: `Content/Materials/Downloaded/PolyHaven/`
   - Select all `.jpg` files
   - Click Open

5. **Organize imported textures:**
   - Move rusty_metal textures → `Materials/PolyHaven/RustyMetal02/`
   - Move metal_plate textures → `Materials/PolyHaven/MetalPlate/`
   - Move concrete_floor textures → `Materials/PolyHaven/ConcreteFloor/`

### Step 2: Configure Texture Settings

**For each Normal Map** (files with `_nor_` in name):
1. Double-click to open
2. **Compression Settings:** `Normalmap (BC5/DXT5)`
3. **sRGB:** ❌ UNCHECKED
4. **Save**

**For each Roughness Map** (files with `_rough_` in name):
1. Double-click to open
2. **Compression Settings:** `Masks (no sRGB)`
3. **sRGB:** ❌ UNCHECKED
4. **Save**

**For each Diffuse/Albedo Map** (files with `_diff_` in name):
1. Double-click to open
2. **Compression Settings:** `Default (DXT1/5)`
3. **sRGB:** ✅ CHECKED
4. **Save**

---

## Creating Materials

### Material 1: M_RustyMetal (Mech Exterior)

1. **Create Material:**
   - Right-click in `Materials/PolyHaven/RustyMetal02/`
   - Create → Material
   - Name: `M_RustyMetal`

2. **Open Material Editor**

3. **Add texture samples:**
   - Drag `rusty_metal_02_diff_2k` into graph
   - Drag `rusty_metal_02_nor_gl_2k` into graph
   - Drag `rusty_metal_02_rough_2k` into graph

4. **Connect nodes:**
   ```
   rusty_metal_02_diff_2k (RGB) → Base Color
   rusty_metal_02_nor_gl_2k (RGB) → Normal
   rusty_metal_02_rough_2k (R) → Roughness
   ```

5. **Set material properties:**
   - Metallic: 0.9 (constant)
   - Specular: 0.5 (constant)

6. **Save & Apply**

### Material 2: M_MetalPlate (Interior/Clean Surfaces)

1. **Create Material:** `M_MetalPlate`

2. **Add textures:**
   - `metal_plate_diff_2k`
   - `metal_plate_nor_gl_2k`
   - `metal_plate_rough_2k`

3. **Connect:**
   ```
   metal_plate_diff_2k (RGB) → Base Color
   metal_plate_nor_gl_2k (RGB) → Normal
   metal_plate_rough_2k (R) → Roughness
   ```

4. **Properties:**
   - Metallic: 1.0
   - Specular: 0.5

5. **Save**

### Material 3: M_ConcreteFloor (Interior Floors)

1. **Create Material:** `M_ConcreteFloor`

2. **Add textures:**
   - `concrete_floor_diff_2k`
   - `concrete_floor_nor_gl_2k`
   - `concrete_floor_rough_2k`

3. **Connect:**
   ```
   concrete_floor_diff_2k (RGB) → Base Color
   concrete_floor_nor_gl_2k (RGB) → Normal
   concrete_floor_rough_2k (R) → Roughness
   ```

4. **Properties:**
   - Metallic: 0.0
   - Specular: 0.1

5. **Save**

---

## Applying Materials to Mech

### Option A: Apply to Procedural Geometry (Quick Test)

Since the mech is currently using procedural geometry, you'll need to modify the C++ code:

**File:** `Source/MechInterior/ProceduralMechGeometry.cpp`

1. Add material loading in constructor:
```cpp
// Load downloaded materials
static ConstructorHelpers::FObjectFinder<UMaterial> RustyMetalMat(TEXT("/Game/Materials/PolyHaven/RustyMetal02/M_RustyMetal"));
if (RustyMetalMat.Succeeded())
{
    MechMaterial = RustyMetalMat.Object;
}
```

2. Apply to mesh components:
```cpp
TorsoMesh->SetMaterial(0, MechMaterial);
HeadMesh->SetMaterial(0, MechMaterial);
```

3. **Rebuild C++ project**

### Option B: Apply in Blueprint (Recommended)

1. **Create Blueprint from C++ class:**
   - Content Browser → C++ Classes → MechInterior
   - Right-click `PlayerMechPawn` → Create Blueprint Class
   - Name: `BP_PlayerMechPawn`

2. **Open Blueprint**

3. **In Construction Script or Details Panel:**
   - Find mesh components (Torso, Head, Legs, Arms)
   - For each component:
     - Materials → Element 0 → Select `M_RustyMetal`

4. **For Interior:**
   - Find Interior components
   - Walls → `M_MetalPlate`
   - Floor → `M_ConcreteFloor`

5. **Compile & Save**

6. **Replace actor in level:**
   - Delete old `PlayerMechPawn`
   - Drag `BP_PlayerMechPawn` into level
   - Position at (0, 0, 0)

### Option C: Apply Directly in Editor

1. **Select mech in level**
2. **Details panel → Components**
3. **For each mesh component:**
   - Expand component
   - Materials → Element 0 → Select material
   - Apply `M_RustyMetal` to exterior
   - Apply `M_MetalPlate` or `M_ConcreteFloor` to interior

---

## Quick Test

1. **Place mech in level** (if not already)
2. **Apply materials** (using one of the methods above)
3. **Press Alt+P** to play
4. **Results:**
   - Mech should now have realistic metal textures
   - Rusty, weathered exterior
   - Clean industrial interior
   - Proper lighting response (PBR materials)

---

## Material Instances (Advanced)

To make it easier to tweak materials without editing the base:

1. **Right-click** on `M_RustyMetal`
2. **Create Material Instance**
3. **Name:** `MI_MechTorso`

4. **Add parameters to base material:**
   - Open `M_RustyMetal`
   - Add `Scalar Parameter` nodes for:
     - `MetallicStrength` (default 0.9)
     - `RoughnessMultiplier` (default 1.0)
   - Add `Vector Parameter` for:
     - `ColorTint` (default white)
   - Multiply ColorTint with diffuse texture before Base Color
   - Multiply RoughnessMultiplier with roughness texture
   - Use MetallicStrength directly for Metallic

5. **In Material Instance:**
   - Tweak parameters in real-time
   - No recompile needed
   - Create variations:
     - `MI_MechTorso_Rusty` (more rust)
     - `MI_MechLegs_Clean` (less rust)
     - `MI_MechHead_Dark` (darker tint)

---

## Troubleshooting

### Materials look flat/wrong:

**Problem:** Normal maps not working
- **Fix:** Make sure normal map texture has `Normalmap (BC5/DXT5)` compression
- **Fix:** Make sure sRGB is UNCHECKED on normal maps

### Materials too shiny/not shiny enough:

**Problem:** Roughness not applied correctly
- **Fix:** Connect roughness to **R channel only** (not RGB)
- **Fix:** Make sure roughness texture has `Masks (no sRGB)` compression

### Materials look washed out:

**Problem:** sRGB settings incorrect
- **Fix:** Diffuse/Albedo should have sRGB CHECKED
- **Fix:** Normal and Roughness should have sRGB UNCHECKED

### Performance issues:

**Problem:** 2k textures too large
- **Solution:** Download 1k versions instead
- **Solution:** Use texture streaming (LOD Bias)

---

## Next Steps

1. ✅ **Test materials on mech** - See how they look
2. **Download more materials** if needed:
   - Scratched metal for weapon systems
   - Carbon fiber for high-tech areas
   - Worn paint for details
3. **Create material variations:**
   - Different rust levels
   - Color variations (blue, red tints)
   - Damage overlays
4. **Add detail textures:**
   - Scratches
   - Decals (warning stripes, numbers)
   - Grime/wear masks

---

## Material Library Summary

| Material | Best For | Metallic | Roughness | Notes |
|----------|----------|----------|-----------|-------|
| M_RustyMetal | Torso, legs, arms | 0.9 | Variable | Battle-worn look |
| M_MetalPlate | Interior walls, panels | 1.0 | Variable | Clean industrial |
| M_ConcreteFloor | Floors, platforms | 0.0 | High | Non-metallic |

---

## Poly Haven Credit

These materials are from **Poly Haven** (polyhaven.com)
- License: CC0 (Public Domain)
- No attribution required (but appreciated!)
- Free for commercial use

---

**Ready to apply!** Start with Option B (Blueprint) for easiest testing.
