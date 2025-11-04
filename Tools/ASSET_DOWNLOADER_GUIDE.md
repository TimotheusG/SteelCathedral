# Asset Downloader Guide

Multi-source asset downloader for SteelCathedrals project. Downloads materials, models, and audio from multiple free sources.

## Supported Sources

### 1. Poly Haven (No API Key Required)
- **Type:** PBR Materials & HDRIs
- **License:** CC0 (Public Domain)
- **Status:** ✅ Enabled by default
- **Use for:** Metal textures, concrete, industrial materials, environment lighting

### 2. Sketchfab (API Key Required)
- **Type:** 3D Models
- **License:** CC0, CC-BY, CC-BY-SA
- **Status:** ⚠️ Disabled (requires API key)
- **Use for:** Machinery, props, mechanical parts, sci-fi assets

### 3. Freesound (API Key Required)
- **Type:** Audio/SFX
- **License:** CC0, CC-BY
- **Status:** ⚠️ Disabled (requires API key)
- **Use for:** Hydraulic sounds, alarms, mechanical effects, ambient audio

---

## Quick Start

### 1. Basic Setup (Poly Haven Only)
No configuration needed! Just run:

```bash
py download_assets.py
```

This downloads all configured Poly Haven materials and HDRIs to:
- `Content/Materials/Downloaded/PolyHaven/`
- `Content/HDRI/Downloaded/PolyHaven/`

### 2. Full Setup (All Sources)

#### Step 1: Get API Keys

**Sketchfab:**
1. Go to https://sketchfab.com/settings/password
2. Scroll to "API & Downloads"
3. Copy your API token

**Freesound:**
1. Go to https://freesound.org/apiv2/apply/
2. Apply for API credentials (instant approval)
3. Copy your API key

#### Step 2: Configure API Keys

Edit `asset_config.json`:

```json
{
  "api_keys": {
    "sketchfab": "your_sketchfab_token_here",
    "freesound": "your_freesound_api_key_here"
  }
}
```

#### Step 3: Enable Sources

In `asset_config.json`, set `enabled: true`:

```json
{
  "sketchfab": {
    "enabled": true,
    "searches": [
      {"query": "industrial machinery", "license": "cc0", "count": 5}
    ]
  },
  "freesound": {
    "enabled": true,
    "searches": [
      {"query": "hydraulic hiss", "count": 5}
    ]
  }
}
```

#### Step 4: Run Full Download

```bash
py download_assets.py
```

---

## Usage Examples

### Discovery & Search

**List all Poly Haven textures:**
```bash
py download_assets.py --list
```

**Search for specific materials:**
```bash
py download_assets.py --search metal
py download_assets.py --search concrete --type textures
```

**Find similar material names:**
```bash
py download_assets.py --similar scratched_metal
```

**Search Sketchfab (requires API key):**
```bash
py download_assets.py --search-sketchfab "reactor core" --count 10
py download_assets.py --search-sketchfab "hydraulic piston" --license cc0
```

**Search Freesound (requires API key):**
```bash
py download_assets.py --search-freesound "alarm siren" --count 5
```

### Download Operations

**Download everything (enabled sources only):**
```bash
py download_assets.py
```

**Suggest alternatives for missing materials:**
```bash
py download_assets.py --suggest
```

---

## Configuration Reference

### `asset_config.json` Structure

```json
{
  "api_keys": {
    "sketchfab": "your_token",
    "freesound": "your_key"
  },

  "polyhaven": {
    "enabled": true,
    "resolution": "2k",        // Options: 1k, 2k, 4k, 8k
    "hdri_resolution": "1k"    // Lower res for performance
  },

  "sketchfab": {
    "enabled": false,
    "searches": [
      {
        "query": "industrial machinery",
        "license": "cc0",      // Options: cc0, cc-by, cc-by-sa
        "count": 5             // Models per search
      }
    ]
  },

  "freesound": {
    "enabled": false,
    "searches": [
      {
        "query": "hydraulic hiss",
        "count": 5             // Sounds per search
      }
    ]
  }
}
```

### Adding Custom Searches

**For Sketchfab models:**
1. Search manually at https://sketchfab.com/
2. Add promising queries to `asset_config.json`:
```json
{
  "query": "sci-fi console",
  "license": "cc-by",
  "count": 3
}
```

**For Freesound audio:**
1. Test searches at https://freesound.org/
2. Add to config:
```json
{
  "query": "metal impact",
  "count": 10
}
```

---

## Output Structure

```
Content/
├── Materials/Downloaded/PolyHaven/
│   ├── scratched_metal/
│   │   ├── scratched_metal_diff_2k.jpg
│   │   ├── scratched_metal_nor_gl_2k.jpg
│   │   └── scratched_metal_rough_2k.jpg
│   └── ...
│
├── HDRI/Downloaded/PolyHaven/
│   ├── industrial_sunset_02/
│   │   └── industrial_sunset_02_1k.hdr
│   └── ...
│
├── Meshes/Downloaded/Sketchfab/
│   ├── author_name/
│   │   ├── Industrial_Machine.zip
│   │   └── Industrial_Machine_metadata.json
│   └── ...
│
└── Audio/Downloaded/Freesound/
    ├── username/
    │   ├── Hydraulic_Sound.wav
    │   └── Hydraulic_Sound_metadata.json
    └── ...
```

---

## Attribution & Licensing

### Automatic Attribution Tracking

The script automatically saves metadata files with attribution info:

**For Sketchfab models:**
```json
{
  "name": "Industrial Pump",
  "author": "author_username",
  "license": "CC-BY 4.0",
  "url": "https://sketchfab.com/3d-models/...",
  "attribution": "Industrial Pump by author_username (https://...)"
}
```

**For Freesound audio:**
```json
{
  "name": "Hydraulic Hiss",
  "author": "sound_creator",
  "license": "CC0",
  "url": "https://freesound.org/...",
  "attribution": "Hydraulic Hiss by sound_creator (https://...)"
}
```

### License Compliance

**CC0 (Public Domain):**
- No attribution required
- Can use commercially
- Can modify freely

**CC-BY (Attribution):**
- Must credit the author
- Use metadata files for proper attribution
- Include credits in game/documentation

**CC-BY-SA (Attribution-ShareAlike):**
- Must credit the author
- Derivative works must use same license
- Carefully review if using for commercial projects

---

## Importing to Unreal Engine

### Materials (Poly Haven)

1. Open UE5 Content Browser
2. Navigate to `Content/Materials/Downloaded/PolyHaven/`
3. Right-click texture → "Create Material"
4. Or use the auto-generated C++ loader code in `material_loading_code.txt`

### Models (Sketchfab)

1. Extract `.zip` files
2. Import `.gltf` or `.fbx` to UE5:
   - Content Browser → Import
   - Select file from extracted folder
   - UE5 handles materials automatically

### Audio (Freesound)

1. UE5 Content Browser → `Content/Audio/Downloaded/Freesound/`
2. Drag `.wav` files into Content Browser
3. UE5 creates Sound Wave assets automatically

---

## Troubleshooting

### "API key not configured"
- Edit `asset_config.json`
- Add your API keys to the `api_keys` section
- Save and rerun

### "No downloadable format available" (Sketchfab)
- Author disabled downloads for this model
- Search for alternatives
- Try adjusting license filter

### "Failed to download" errors
- Check internet connection
- Verify API keys are valid
- Check if rate limiting (wait 1-2 minutes)
- Some assets may be region-restricted

### Downloads are slow
- Normal! High-quality assets are large
- 2K textures: 5-15 MB each
- Models: 10-100+ MB
- Audio: 1-10 MB
- Consider lowering resolution in config

### Missing materials after download
- Run: `py download_assets.py --suggest`
- Script will find alternatives
- Edit `download_assets.py` to add alternatives to `POLYHAVEN_MATERIALS`

---

## Best Practices

### 1. Start Small
- Test with a few searches first
- Set `count: 2-3` initially
- Enable one source at a time
- Verify quality before batch downloading

### 2. Quality Control
- Preview assets on source websites first
- Filter for game-ready topology (Sketchfab)
- Check poly counts before importing
- Test materials in UE5 before committing

### 3. License Management
- Keep metadata files with assets
- Document attributions in project notes
- Use CC0 for commercial projects when possible
- Review CC-BY licenses carefully

### 4. Storage Management
- 2K textures recommended (good quality, reasonable size)
- Use 1K HDRIs (sufficient for background lighting)
- Delete unused assets to save space
- Consider Git LFS for team projects

### 5. Performance
- Lower resolutions for background objects
- Use 4K/8K only for hero assets
- Compress audio to match UE5 requirements
- Use texture streaming in UE5

---

## Recommended Asset Categories

### For Mech Interiors:
**Materials:**
- scratched_metal, brushed_metal, painted_metal
- metal_plate, steel_plate, diamond_plate
- industrial_panel, grated_metal_floor
- rubber_tiles, cable_bundle

**Models (Sketchfab):**
- "hydraulic piston", "mechanical arm"
- "reactor core", "sci-fi console"
- "industrial machinery", "control panel"
- "metal grating", "pipes junction"

**Audio (Freesound):**
- "hydraulic hiss", "metal creak"
- "alarm siren", "warning beep"
- "reactor hum", "electrical buzz"
- "steam release", "footstep metal"

### For Exterior/Environment:
**Materials:**
- worn_concrete, rough_concrete, damaged_concrete
- dirt, grime, rust
- weathered_metal, corroded_metal

**HDRIs:**
- industrial_sunset, wasteland_clouds
- abandoned_hall, urban ruins

---

## Advanced Usage

### Custom Material Lists

Edit `download_assets.py` lines 35-75 to add materials:

```python
POLYHAVEN_MATERIALS = [
    "your_custom_material",
    "another_material",
    # ...
]
```

### Batch Operations

Run searches and save results:
```bash
py download_assets.py --search "metal" > metal_materials.txt
```

Review and add to config.

### Integration with Git

Add to `.gitignore`:
```
Content/Materials/Downloaded/
Content/Meshes/Downloaded/
Content/Audio/Downloaded/
*.log
```

Or use Git LFS for team sharing:
```
git lfs track "Content/Materials/Downloaded/**/*.jpg"
```

---

## Future Enhancements

Potential additions (not yet implemented):
- [ ] TurboSquid/CGTrader API integration
- [ ] Quixel Bridge CLI automation
- [ ] Unity Asset Store downloader
- [ ] Automatic UE5 import via Python API
- [ ] Duplicate detection
- [ ] Asset quality scoring

---

## Support & Resources

**Poly Haven:**
- Website: https://polyhaven.com/
- No account needed
- 100% free CC0 assets

**Sketchfab:**
- Website: https://sketchfab.com/
- API Docs: https://docs.sketchfab.com/data-api/
- Free account required

**Freesound:**
- Website: https://freesound.org/
- API Docs: https://freesound.org/docs/api/
- Free account required

**Issues & Questions:**
- Check logs: `asset_download.log`
- Review metadata files in download folders
- Test API keys in browser first
- Verify source website is accessible

---

## License

This downloader script is provided as-is for the SteelCathedrals project.

**Downloaded assets retain their original licenses:**
- Poly Haven: CC0 (no attribution required)
- Sketchfab: Varies (check metadata files)
- Freesound: Varies (check metadata files)

Always review and comply with asset licenses before commercial use.
