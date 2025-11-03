# How to Get API Keys (Optional)

Poly Haven works without API keys! But if you want 3D models and audio, follow these steps:

---

## Sketchfab (3D Models)

### Step 1: Get the API Key
1. Go to: **https://sketchfab.com/settings/password**
2. Log in (or create a free account)
3. Scroll down to the **"API & Downloads"** section
4. Copy your **API Token**

### Step 2: Add to Config
1. Open `asset_config.json`
2. Paste your token:
```json
{
  "api_keys": {
    "sketchfab": "paste_your_token_here"
  }
}
```

### Step 3: Enable Downloads
In `asset_config.json`:
```json
{
  "sketchfab": {
    "enabled": true
  }
}
```

### Step 4: Test It
```bash
py download_assets.py --search-sketchfab "reactor core"
```

---

## Freesound (Audio/SFX)

### Step 1: Get the API Key
1. Go to: **https://freesound.org/apiv2/apply/**
2. Log in (or create a free account)
3. Fill out the simple form (name, description)
4. **Instant approval!** Copy your API key

### Step 2: Add to Config
1. Open `asset_config.json`
2. Paste your key:
```json
{
  "api_keys": {
    "freesound": "paste_your_key_here"
  }
}
```

### Step 3: Enable Downloads
In `asset_config.json`:
```json
{
  "freesound": {
    "enabled": true
  }
}
```

### Step 4: Test It
```bash
py download_assets.py --search-freesound "hydraulic hiss"
```

---

## Complete Example Config

```json
{
  "api_keys": {
    "sketchfab": "a1b2c3d4e5f6...",
    "freesound": "x1y2z3..."
  },

  "sketchfab": {
    "enabled": true,
    "searches": [
      {"query": "industrial machinery", "license": "cc0", "count": 5},
      {"query": "reactor core", "license": "cc0", "count": 3}
    ]
  },

  "freesound": {
    "enabled": true,
    "searches": [
      {"query": "hydraulic hiss", "count": 5},
      {"query": "metal creak", "count": 5},
      {"query": "alarm siren", "count": 3}
    ]
  },

  "polyhaven": {
    "enabled": true,
    "resolution": "2k",
    "hdri_resolution": "1k"
  }
}
```

---

## Then Download Everything

```bash
py download_assets.py
```

It will download:
- ✅ Poly Haven materials (already working!)
- ✅ Sketchfab models (if API key added)
- ✅ Freesound audio (if API key added)

---

## Don't Want to Set Up API Keys?

**No problem!** Poly Haven already downloaded:
- 16 metal textures
- 10 concrete textures
- 2 HDRIs
- 123.9 MB total

That's plenty to get started! You can add Sketchfab/Freesound later.

---

## Quick Links

- **Sketchfab API:** https://sketchfab.com/settings/password
- **Freesound API:** https://freesound.org/apiv2/apply/
- **Poly Haven:** https://polyhaven.com/ (no key needed!)
