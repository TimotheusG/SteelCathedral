# Asset Discovery Guide

The `download_assets.py` script now includes powerful search and discovery features to help find materials on Poly Haven.

## 📚 Quick Reference

### List All Available Assets
```bash
# List all textures (default)
py download_assets.py --list

# List all HDRIs
py download_assets.py --list --type hdris
```

**Output:** Complete catalog of available assets with categories

---

### Search for Materials
```bash
# Search for metal materials
py download_assets.py --search metal

# Search for concrete materials
py download_assets.py --search concrete

# Search for industrial materials
py download_assets.py --search industrial
```

**Output:** All materials matching the keyword in name, categories, or tags

**Example Results:**
```
✅ Found 26 matches:
  1. blue_metal_plate               [metal, industrial]
  2. corrugated_iron                [man made, roofing, wall]
  3. metal_plate                    [metal, floor, man made]
  4. rusty_metal_02                 [metal, man made, dirty]
  ...
```

---

### Find Similar Names (Fuzzy Matching)
```bash
# Find materials similar to "scratched_metal"
py download_assets.py --similar scratched_metal

# Find top 5 similar (default)
py download_assets.py --similar brushed_metal

# Find top 10 similar
py download_assets.py --similar carbon_fiber --count 10
```

**Output:** Materials with similar names ranked by similarity percentage

**Example Results:**
```
✅ Found 3 similar names:
  1. rusty_painted_metal            (similarity: 64.71%) [man made, metal]
  2. rusty_metal                    (similarity: 61.54%) [metal, man made]
  3. cracked_concrete_wall          (similarity: 61.11%) [outdoor, man made]
```

---

### Suggest Alternatives for Missing Materials
```bash
# Automatically find alternatives for all missing materials
py download_assets.py --suggest
```

**What it does:**
1. Checks which materials from `POLYHAVEN_MATERIALS` list weren't downloaded
2. For each missing material:
   - Searches by keyword first
   - Falls back to fuzzy name matching
3. Generates `material_alternatives.txt` with all suggestions

**Output File Format:**
```markdown
# Material Alternatives for Steel Cathedral

## scratched_metal
Suggested alternatives:
  - rusty_painted_metal
  - rusty_metal
  - cracked_concrete_wall

## painted_metal_01
Suggested alternatives:
  - painted_metal_shutter
  - rusty_painted_metal
  - painted_concrete_02

## carbon_fiber
  No alternatives found - try manual search
```

---

## 🔍 Real-World Examples

### Problem: Material Not Found
```bash
# We tried to download "scratched_metal" but it doesn't exist
# Let's find what's actually available:

py download_assets.py --similar scratched_metal
```

**Result:**
```
✅ Found 3 similar names:
  1. rusty_painted_metal (64.71% match)
  2. rusty_metal (61.54% match)
  3. cracked_concrete_wall (61.11% match)
```

**Action:** Update `POLYHAVEN_MATERIALS` list to use `rusty_painted_metal` instead

---

### Problem: Don't Know What's Available
```bash
# Let's see all metal materials:
py download_assets.py --search metal
```

**Result:** 26 metal materials listed with categories

**Action:** Pick the ones that fit our mech aesthetic

---

### Problem: Many Materials Missing
```bash
# Let's find alternatives for everything at once:
py download_assets.py --suggest
```

**Result:** `material_alternatives.txt` created with all suggestions

**Action:** Review file and update `POLYHAVEN_MATERIALS` list

---

## 🎯 Workflow for Fixing Missing Materials

### Step 1: Run Normal Download
```bash
py download_assets.py
```

**Check output** for materials that weren't found (⚠️ warnings)

### Step 2: Generate Suggestions
```bash
py download_assets.py --suggest
```

**Review** `material_alternatives.txt` for recommendations

### Step 3: Search Manually (if needed)
```bash
# If suggestions aren't good, search by keyword:
py download_assets.py --search metal
py download_assets.py --search concrete
py download_assets.py --search industrial
```

### Step 4: Update Material List
Edit `download_assets.py` and update `POLYHAVEN_MATERIALS` array:

```python
POLYHAVEN_MATERIALS = [
    # OLD (doesn't exist)
    # "scratched_metal",

    # NEW (found via --similar)
    "rusty_painted_metal",

    # ... rest of materials
]
```

### Step 5: Re-run Download
```bash
py download_assets.py
```

---

## 📊 Search Techniques

### Keyword Search (Broad)
Best for: Finding all materials in a category
```bash
py download_assets.py --search metal
py download_assets.py --search rust
py download_assets.py --search industrial
```

Searches in:
- Material name
- Categories (metal, industrial, outdoor, etc.)
- Tags (man made, dirty, clean, etc.)

### Fuzzy Matching (Precise)
Best for: Finding replacements for specific material names
```bash
py download_assets.py --similar scratched_metal
py download_assets.py --similar brushed_metal
py download_assets.py --similar diamond_plate
```

Uses: String similarity algorithm (difflib)
Returns: Materials ranked by name similarity percentage

### Listing (Discovery)
Best for: Browsing entire catalog
```bash
py download_assets.py --list
py download_assets.py --list --type hdris
```

Returns: Complete sorted list of all available assets

---

## 🔬 How Fuzzy Matching Works

The script uses Python's `difflib.SequenceMatcher` to calculate similarity:

```python
similarity = SequenceMatcher(None, "scratched_metal", "rusty_metal").ratio()
# Result: 0.6154 (61.54% match)
```

**Similarity Scores:**
- 90-100%: Very close match (typos, plurals)
- 70-89%: Good match (similar words)
- 50-69%: Moderate match (some common words)
- 30-49%: Weak match (might be relevant)
- <30%: Not shown (too dissimilar)

**Minimum cutoff:** 30% (configurable in code)

---

## 🎨 Finding Materials for Steel Cathedral

### Mech Exterior (Metals)
```bash
py download_assets.py --search metal | grep -i "plate\|rust\|corrugated\|industrial"
```

Good materials:
- `metal_plate`, `metal_plate_02`
- `rusty_metal_02` through `rusty_metal_05`
- `corrugated_iron`, `corrugated_iron_02`, `corrugated_iron_03`
- `blue_metal_plate`
- `container_side`
- `factory_wall`

### Interior Panels
```bash
py download_assets.py --search industrial
```

Good materials:
- `factory_wall`
- `metal_grate_rusty`
- `rusty_metal_grid`
- `worn_shutter`

### Concrete/Structure
```bash
py download_assets.py --search concrete
```

Good materials:
- `concrete_wall_001` through `concrete_wall_008`
- `concrete_floor_worn`
- `damaged_concrete`
- `concrete_cracked`

### Weathering/Grime
```bash
py download_assets.py --search dirt
py download_assets.py --search grime
py download_assets.py --search rust
```

Good materials:
- `dirt_01` through `dirt_05`
- `rust_coarse_01`
- `grunge_wall`
- `worn_painted_metal`

---

## 💡 Pro Tips

### 1. Browse Before Downloading
Always explore what's available before committing to a material list:
```bash
py download_assets.py --list > all_materials.txt
# Review all_materials.txt to pick materials
```

### 2. Use Suggest First
Let the script find alternatives automatically:
```bash
py download_assets.py --suggest
# Review material_alternatives.txt
# Update POLYHAVEN_MATERIALS list
# Re-run download
```

### 3. Search Multiple Keywords
Combine searches to find the perfect material:
```bash
py download_assets.py --search metal
py download_assets.py --search rust
py download_assets.py --search industrial
```

### 4. Check Categories
Pay attention to categories in search results - they help identify material use:
- `[metal, floor]` - Good for flooring
- `[wall, outdoor]` - Good for exterior walls
- `[indoor, clean]` - Good for clean interior
- `[dirty, man made]` - Good for weathered surfaces

### 5. Similarity Threshold
Higher similarity = more likely to look similar:
- 70%+ similarity → Very likely to be a good replacement
- 50-70% similarity → Maybe similar, check visually
- <50% similarity → Probably different aesthetic

---

## 🚀 Advanced Usage

### Batch Search
```bash
# Search for multiple types and combine results
py download_assets.py --search metal > metal_results.txt
py download_assets.py --search concrete > concrete_results.txt
py download_assets.py --search rust > rust_results.txt
```

### Custom Material List
Create a custom list based on search results:

1. Search: `py download_assets.py --search metal`
2. Copy material names from output
3. Paste into `POLYHAVEN_MATERIALS` array in script
4. Run download: `py download_assets.py`

### HDRIs for Lighting
```bash
# Find environment HDRIs
py download_assets.py --list --type hdris | grep -i "industrial\|sunset\|wasteland"

# Find specific HDRI
py download_assets.py --search industrial --type hdris
```

---

## 📝 Material Categories

Common categories you'll see:

**Environment:**
- `outdoor` - Suitable for exterior
- `indoor` - Suitable for interior
- `wall` - Wall textures
- `floor` - Floor textures
- `roofing` - Roof materials

**Material Type:**
- `metal` - Metal surfaces
- `concrete` - Concrete/cement
- `fabric` - Cloth materials
- `wood` - Wooden surfaces
- `plastic` - Plastic materials

**Condition:**
- `clean` - Clean, pristine
- `dirty` - Dirty, weathered
- `worn` - Worn, aged
- `damaged` - Damaged, broken

**Purpose:**
- `man made` - Artificial materials
- `industrial` - Industrial/factory
- `natural` - Natural materials

---

## 🔗 Poly Haven API

The script uses these endpoints:

**List Assets:**
```
GET https://api.polyhaven.com/assets?t=textures
GET https://api.polyhaven.com/assets?t=hdris
```

**Get Asset Info:**
```
GET https://api.polyhaven.com/files/{material_name}
```

**Documentation:** https://api.polyhaven.com/

---

## ⚠️ Troubleshooting

### "Failed to fetch asset list"
- **Cause:** Network error or API down
- **Fix:** Check internet connection, try again later

### "No matches found"
- **Cause:** Keyword too specific or misspelled
- **Fix:** Try broader keywords (e.g., "metal" instead of "scratched_metal")

### "No similar names found"
- **Cause:** Material name very unusual
- **Fix:** Use `--search` instead of `--similar`

### Script hangs during --suggest
- **Cause:** Processing 26+ materials takes time (API rate limiting)
- **Fix:** Wait for completion (~30 seconds), output goes to file

---

**Last Updated:** 2025-11-03
