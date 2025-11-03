# Poly Haven Materials Guide

## Overview

`download_assets.py` pulls CC0 materials from [Poly Haven](https://polyhaven.com) and prepares them for the project. This note captures the handful of commands we actually use plus the current curated library.

---

## Quick Start

```bash
py download_assets.py
```

- Downloads the default 2K texture set (diffuse, normal, roughness) and five HDRIs.
- Drops files into `Content/Materials/Downloaded/PolyHaven/`.
- Unreal imports them on launch; `UProceduralMechGeometry::LoadDownloadedMaterials()` builds dynamic material instances automatically.

If the editor still shows grey materials, re-run the script and restart the editor so auto-import runs.

---

## Discover Materials Fast

| Goal | Command | Notes |
|------|---------|-------|
| List supported textures | `py download_assets.py --list` | Add `--type hdris` to list skyboxes. |
| Keyword search | `py download_assets.py --search metal` | Matches names, categories, and tags. |
| Fuzzy match | `py download_assets.py --similar scratched_metal` | Optional `--count 10` for more hits. |
| Suggest replacements | `py download_assets.py --suggest` | Writes `material_alternatives.txt` summarising substitutes. |

Thumb rules:
- Similarity 70%+ is usually a drop-in replacement.
- Between 50% and 70% deserves a visual check.
- Below 50% expect a different look.

---

## Curated Library

Adjust `POLYHAVEN_MATERIALS` as art direction shifts; this is the current set.

**Exterior Metals**
- `rusty_metal_02` (default hull)
- `metal_plate`
- `corrugated_iron`
- `brushed_metal`
- `diamond_plate`
- `titanium_scuffed`
- `weathered_metal`

**Interior and Structure**
- `metal_grill`
- `industrial_flooring`
- `factory_wall`
- `rough_metal_plates`
- `concrete_floor`
- `damaged_concrete`
- `concrete_blocks`
- `rough_concrete_01`

**Detail Accents**
- `rubber_tiles_01`
- `cable_bundle`
- `dirt_01`
- `grime_01`

---

## Troubleshooting

- **Materials render grey**: textures never imported. Re-run the script and restart the editor, or import the JPGs manually.
- **Normals look wrong**: set Compression to "Normal map (BC5/DXT5)" and disable sRGB on the normal texture.
- **Too glossy or too dull**: confirm the roughness texture uses the R channel only and "Masks" compression.
- **Downloads fail**: Poly Haven may rate-limit during `--suggest`. Wait ~30 seconds and retry.

---

## Reference

- API endpoints: `https://api.polyhaven.com/assets?t=textures`, `https://api.polyhaven.com/files/<asset_name>`
- Script location: project root `download_assets.py`
- Licence: CC0, so use, modify, or redistribute without attribution.

Keep this file handy; it should answer nearly every Poly Haven question.
