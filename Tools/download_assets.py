#!/usr/bin/env python3
"""
Asset Downloader for Steel Cathedral
Downloads materials from Poly Haven and models from Sketchfab
Runs in background, logs progress
"""

import requests
import os
import json
import time
import sys
import argparse
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
from difflib import SequenceMatcher, get_close_matches

# Fix Windows encoding issues
if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8')
    sys.stderr.reconfigure(encoding='utf-8')

# Configuration
PROJECT_ROOT = Path(__file__).parent
POLYHAVEN_DIR = PROJECT_ROOT / "Content" / "Materials" / "Downloaded" / "PolyHaven"
HDRI_DIR = PROJECT_ROOT / "Content" / "HDRI" / "Downloaded" / "PolyHaven"
SKETCHFAB_DIR = PROJECT_ROOT / "Content" / "Meshes" / "Downloaded" / "Sketchfab"
FREESOUND_DIR = PROJECT_ROOT / "Content" / "Audio" / "Downloaded" / "Freesound"
LOG_FILE = PROJECT_ROOT / "asset_download.log"
CONFIG_FILE = PROJECT_ROOT / "asset_config.json"

# Resolution to download (2k is good balance, 1k for HDRIs)
RESOLUTION = "2k"
HDRI_RESOLUTION = "1k"

# Materials to download from Poly Haven
POLYHAVEN_MATERIALS = [
    # Metals (mech exterior)
    "scratched_metal",
    "metal_plate",
    "rusty_metal",
    "rusty_metal_02",
    "corrugated_iron",
    "green_metal_rust",
    "blue_metal_plate",

    # Industrial (interior panels)
    "metal_grate_rusty",
    "factory_wall",

    # Concrete/Structure (terrain, ruins)
    "concrete_wall_001",
    "concrete_floor",
    "rough_concrete",
    "damaged_concrete_floor",
    "cracked_concrete",
    "brushed_concrete",

    # Dirt/Grime (weathering)
    "dirty_concrete",
]

# HDRIs for lighting (smaller 1k for performance)
POLYHAVEN_HDRIS = [
    "industrial_sunset_02",
    "wasteland_clouds",
]

class AssetDownloader:
    def __init__(self):
        self.log_file = open(LOG_FILE, 'a', encoding='utf-8', buffering=1)
        self.downloaded_count = 0
        self.failed_count = 0
        self.config = self.load_config()

    def log(self, message):
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        log_msg = f"[{timestamp}] {message}"
        print(log_msg)
        self.log_file.write(log_msg + "\n")

    def load_config(self):
        """Load configuration from JSON file"""
        if CONFIG_FILE.exists():
            try:
                with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
                    return json.load(f)
            except Exception as e:
                print(f"⚠️  Failed to load config: {e}")
                return {}
        return {}

    def download_file(self, url, dest_path):
        """Download a file with progress"""
        try:
            self.log(f"Downloading {dest_path.name}...")
            response = requests.get(url, stream=True, timeout=60)
            response.raise_for_status()

            total_size = int(response.headers.get('content-length', 0))

            with open(dest_path, 'wb') as f:
                downloaded = 0
                for chunk in response.iter_content(chunk_size=8192):
                    if chunk:
                        f.write(chunk)
                        downloaded += len(chunk)

            self.log(f"✅ Downloaded {dest_path.name} ({downloaded / 1024 / 1024:.1f} MB)")
            return True

        except Exception as e:
            self.log(f"❌ Failed to download {dest_path.name}: {e}")
            return False

    def get_polyhaven_material_info(self, material_name):
        """Get download URLs for a Poly Haven material"""
        try:
            url = f"https://api.polyhaven.com/files/{material_name}"
            response = requests.get(url, timeout=10)

            if response.status_code == 404:
                self.log(f"⚠️  Material '{material_name}' not found")
                return None

            response.raise_for_status()
            return response.json()

        except Exception as e:
            self.log(f"❌ Failed to get info for {material_name}: {e}")
            return None

    def download_polyhaven_material(self, material_name):
        """Download complete material set from Poly Haven"""
        self.log(f"\n{'='*60}")
        self.log(f"Downloading: {material_name}")
        self.log(f"{'='*60}")

        # Get material info
        material_info = self.get_polyhaven_material_info(material_name)
        if not material_info:
            self.failed_count += 1
            return False

        # Create material directory
        material_dir = POLYHAVEN_DIR / material_name
        material_dir.mkdir(parents=True, exist_ok=True)

        # Download maps we need: Diffuse, Normal, Roughness
        maps_to_download = {
            'Diffuse': f'{material_name}_diff_{RESOLUTION}.jpg',
            'nor_gl': f'{material_name}_nor_gl_{RESOLUTION}.jpg',
            'Rough': f'{material_name}_rough_{RESOLUTION}.jpg',
        }

        success = True
        for map_type, filename in maps_to_download.items():
            try:
                # Navigate JSON structure
                if map_type in material_info:
                    map_data = material_info[map_type]
                elif map_type.lower() in material_info:
                    map_data = material_info[map_type.lower()]
                else:
                    self.log(f"⚠️  No {map_type} map available")
                    continue

                # Get resolution
                if RESOLUTION in map_data:
                    res_data = map_data[RESOLUTION]
                else:
                    self.log(f"⚠️  Resolution {RESOLUTION} not available for {map_type}")
                    continue

                # Get JPG URL
                if 'jpg' in res_data:
                    url = res_data['jpg']['url']
                    dest_path = material_dir / filename

                    if dest_path.exists():
                        self.log(f"⏭️  Skipping {filename} (already exists)")
                    else:
                        if not self.download_file(url, dest_path):
                            success = False
                        else:
                            self.downloaded_count += 1

            except Exception as e:
                self.log(f"❌ Error downloading {map_type}: {e}")
                success = False

        if not success:
            self.failed_count += 1

        return success

    def download_all_polyhaven(self):
        """Download all configured Poly Haven materials"""
        self.log("\n" + "="*60)
        self.log("STARTING POLY HAVEN DOWNLOAD")
        self.log("="*60)
        self.log(f"Materials to download: {len(POLYHAVEN_MATERIALS)}")
        self.log(f"Resolution: {RESOLUTION}")
        self.log(f"Destination: {POLYHAVEN_DIR}")

        POLYHAVEN_DIR.mkdir(parents=True, exist_ok=True)

        for material in POLYHAVEN_MATERIALS:
            self.download_polyhaven_material(material)
            time.sleep(0.5)  # Be nice to the API

    def download_polyhaven_hdri(self, hdri_name):
        """Download HDRI from Poly Haven"""
        self.log(f"\n{'='*60}")
        self.log(f"Downloading HDRI: {hdri_name}")
        self.log(f"{'='*60}")

        # Get HDRI info
        hdri_info = self.get_polyhaven_material_info(hdri_name)
        if not hdri_info:
            self.failed_count += 1
            return False

        # Create HDRI directory
        hdri_dir = HDRI_DIR / hdri_name
        hdri_dir.mkdir(parents=True, exist_ok=True)

        # Download HDR file
        try:
            if 'hdri' in hdri_info:
                hdri_data = hdri_info['hdri']
                if HDRI_RESOLUTION in hdri_data:
                    res_data = hdri_data[HDRI_RESOLUTION]
                    if 'hdr' in res_data:
                        url = res_data['hdr']['url']
                        dest_path = hdri_dir / f'{hdri_name}_{HDRI_RESOLUTION}.hdr'

                        if dest_path.exists():
                            self.log(f"⏭️  Skipping {hdri_name} (already exists)")
                        else:
                            if self.download_file(url, dest_path):
                                self.downloaded_count += 1
                                return True
                            else:
                                self.failed_count += 1
                                return False
                    else:
                        self.log(f"⚠️  No HDR format available for {hdri_name}")
                else:
                    self.log(f"⚠️  Resolution {HDRI_RESOLUTION} not available for {hdri_name}")
            else:
                self.log(f"⚠️  No HDRI data available for {hdri_name}")

        except Exception as e:
            self.log(f"❌ Error downloading HDRI: {e}")
            self.failed_count += 1
            return False

        return False

    def download_all_hdris(self):
        """Download all configured HDRIs"""
        self.log("\n" + "="*60)
        self.log("STARTING HDRI DOWNLOAD")
        self.log("="*60)
        self.log(f"HDRIs to download: {len(POLYHAVEN_HDRIS)}")
        self.log(f"Resolution: {HDRI_RESOLUTION}")
        self.log(f"Destination: {HDRI_DIR}")

        HDRI_DIR.mkdir(parents=True, exist_ok=True)

        for hdri in POLYHAVEN_HDRIS:
            self.download_polyhaven_hdri(hdri)
            time.sleep(0.5)  # Be nice to the API

    def update_cpp_material_loader(self):
        """Generate C++ code snippet for loading new materials"""
        self.log("\n" + "="*60)
        self.log("GENERATING C++ CODE SNIPPET")
        self.log("="*60)

        cpp_file = PROJECT_ROOT / "material_loading_code.txt"

        with open(cpp_file, 'w', encoding='utf-8') as f:
            f.write("// Add these to ProceduralMechGeometry.cpp LoadDownloadedMaterials():\n\n")

            for material in POLYHAVEN_MATERIALS:
                material_dir = POLYHAVEN_DIR / material
                if material_dir.exists():
                    f.write(f"// {material}\n")
                    f.write(f'UTexture2D* {material}_Diffuse = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/{material}/{material}_diff_{RESOLUTION}"));\n')
                    f.write(f'UTexture2D* {material}_Normal = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/{material}/{material}_nor_gl_{RESOLUTION}"));\n')
                    f.write(f'UTexture2D* {material}_Roughness = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/{material}/{material}_rough_{RESOLUTION}"));\n')
                    f.write(f'if ({material}_Diffuse && {material}_Normal && {material}_Roughness) {{\n')
                    f.write(f'    {material.title().replace("_", "")}Material = CreateMaterialWithTextures({material}_Diffuse, {material}_Normal, {material}_Roughness);\n')
                    f.write(f'    UE_LOG(LogTemp, Warning, TEXT("✅ {material} material loaded"));\n')
                    f.write('}\n\n')

        self.log(f"✅ C++ code snippet written to: {cpp_file}")

    def create_summary(self):
        """Create download summary"""
        self.log("\n" + "="*60)
        self.log("DOWNLOAD SUMMARY")
        self.log("="*60)
        self.log(f"✅ Successfully downloaded: {self.downloaded_count} files")
        self.log(f"❌ Failed/skipped: {self.failed_count}")

        # List what was skipped for next pass
        if self.failed_count > 0:
            self.log("\n📋 Materials to find alternatives for:")
            for material in POLYHAVEN_MATERIALS:
                material_dir = POLYHAVEN_DIR / material
                if not material_dir.exists() or not any(material_dir.glob('*.jpg')):
                    self.log(f"   - {material}")

        # Count total files
        total_textures = sum(1 for _ in POLYHAVEN_DIR.rglob('*.jpg'))
        total_hdris = sum(1 for _ in HDRI_DIR.rglob('*.hdr')) if HDRI_DIR.exists() else 0
        self.log(f"📁 Total texture files: {total_textures}")
        self.log(f"📁 Total HDRI files: {total_hdris}")

        # Calculate size
        texture_size = sum(f.stat().st_size for f in POLYHAVEN_DIR.rglob('*.jpg'))
        hdri_size = sum(f.stat().st_size for f in HDRI_DIR.rglob('*.hdr')) if HDRI_DIR.exists() else 0
        total_size = texture_size + hdri_size
        self.log(f"💾 Textures size: {texture_size / 1024 / 1024:.1f} MB")
        self.log(f"💾 HDRIs size: {hdri_size / 1024 / 1024:.1f} MB")
        self.log(f"💾 Total size: {total_size / 1024 / 1024:.1f} MB")

        self.log("\n✅ DOWNLOAD COMPLETE!")
        self.log(f"📝 Full log: {LOG_FILE}")
        self.log("🔨 Next: Import textures in UE5 Content Browser")

    def list_available_assets(self, asset_type="textures"):
        """List all available assets from Poly Haven"""
        self.log(f"\n🔍 Fetching available {asset_type} from Poly Haven...")

        try:
            url = f"https://api.polyhaven.com/assets?t={asset_type}"
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            assets = response.json()

            asset_names = sorted(assets.keys())
            self.log(f"\n📋 Found {len(asset_names)} {asset_type}:\n")

            for i, name in enumerate(asset_names, 1):
                info = assets[name]
                categories = info.get('categories', [])
                tags = info.get('tags', [])
                self.log(f"  {i:3d}. {name:30s} [{', '.join(categories[:3])}]")

            return asset_names

        except Exception as e:
            self.log(f"❌ Failed to fetch asset list: {e}")
            return []

    def search_assets(self, keyword, asset_type="textures"):
        """Search for assets matching a keyword"""
        self.log(f"\n🔍 Searching for '{keyword}' in {asset_type}...")

        try:
            url = f"https://api.polyhaven.com/assets?t={asset_type}"
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            assets = response.json()

            # Search in name, categories, and tags
            matches = []
            keyword_lower = keyword.lower()

            for name, info in assets.items():
                name_lower = name.lower()
                categories = [c.lower() for c in info.get('categories', [])]
                tags = [t.lower() for t in info.get('tags', [])]

                if (keyword_lower in name_lower or
                    any(keyword_lower in cat for cat in categories) or
                    any(keyword_lower in tag for tag in tags)):
                    matches.append((name, info))

            if matches:
                self.log(f"\n✅ Found {len(matches)} matches:\n")
                for i, (name, info) in enumerate(matches, 1):
                    categories = info.get('categories', [])
                    self.log(f"  {i:3d}. {name:30s} [{', '.join(categories[:3])}]")
            else:
                self.log(f"\n⚠️  No matches found for '{keyword}'")

            return [name for name, _ in matches]

        except Exception as e:
            self.log(f"❌ Failed to search: {e}")
            return []

    def find_similar(self, material_name, asset_type="textures", n=5):
        """Find assets with similar names using fuzzy matching"""
        self.log(f"\n🔍 Finding similar names to '{material_name}'...")

        try:
            url = f"https://api.polyhaven.com/assets?t={asset_type}"
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            assets = response.json()

            asset_names = list(assets.keys())

            # Use difflib for fuzzy matching
            similar = get_close_matches(material_name, asset_names, n=n, cutoff=0.3)

            if similar:
                self.log(f"\n✅ Found {len(similar)} similar names:\n")
                for i, name in enumerate(similar, 1):
                    info = assets[name]
                    categories = info.get('categories', [])
                    # Calculate similarity score
                    ratio = SequenceMatcher(None, material_name, name).ratio()
                    self.log(f"  {i}. {name:30s} (similarity: {ratio:.2%}) [{', '.join(categories[:2])}]")
            else:
                self.log(f"\n⚠️  No similar names found for '{material_name}'")

            return similar

        except Exception as e:
            self.log(f"❌ Failed to find similar: {e}")
            return []

    def suggest_alternatives(self):
        """Suggest alternatives for materials that weren't found"""
        self.log("\n" + "="*60)
        self.log("FINDING ALTERNATIVES FOR MISSING MATERIALS")
        self.log("="*60)

        missing_materials = []
        for material in POLYHAVEN_MATERIALS:
            material_dir = POLYHAVEN_DIR / material
            if not material_dir.exists() or not any(material_dir.glob('*.jpg')):
                missing_materials.append(material)

        if not missing_materials:
            self.log("\n✅ No missing materials!")
            return

        self.log(f"\nFound {len(missing_materials)} missing materials. Searching for alternatives...\n")

        suggestions = {}
        for material in missing_materials:
            self.log(f"\n{'='*60}")
            self.log(f"Material: {material}")
            self.log(f"{'='*60}")

            # Try exact search first
            matches = self.search_assets(material.replace('_', ' '), "textures")
            if not matches:
                # Try fuzzy matching
                matches = self.find_similar(material, "textures", n=3)

            suggestions[material] = matches
            time.sleep(0.5)  # Be nice to the API

        # Write suggestions to file
        suggestions_file = PROJECT_ROOT / "material_alternatives.txt"
        with open(suggestions_file, 'w', encoding='utf-8') as f:
            f.write("# Material Alternatives for Steel Cathedral\n\n")
            for material, alternatives in suggestions.items():
                f.write(f"## {material}\n")
                if alternatives:
                    f.write("Suggested alternatives:\n")
                    for alt in alternatives:
                        f.write(f"  - {alt}\n")
                else:
                    f.write("  No alternatives found - try manual search\n")
                f.write("\n")

        self.log(f"\n✅ Suggestions written to: {suggestions_file}")

    def search_sketchfab_models(self, query, license="cc0", count=5):
        """Search for models on Sketchfab"""
        api_key = self.config.get('api_keys', {}).get('sketchfab', '')
        if not api_key:
            self.log("⚠️  Sketchfab API key not configured in asset_config.json")
            return []

        try:
            url = "https://api.sketchfab.com/v3/models"
            headers = {"Authorization": f"Token {api_key}"}
            params = {
                "q": query,
                "license": license,
                "animated": "false",
                "count": count,
                "downloadable": "true"
            }

            self.log(f"🔍 Searching Sketchfab for: {query} (license: {license})")
            response = requests.get(url, headers=headers, params=params, timeout=30)
            response.raise_for_status()

            results = response.json().get('results', [])
            self.log(f"✅ Found {len(results)} models")

            return results

        except Exception as e:
            self.log(f"❌ Failed to search Sketchfab: {e}")
            return []

    def download_sketchfab_model(self, model):
        """Download a single model from Sketchfab"""
        api_key = self.config.get('api_keys', {}).get('sketchfab', '')
        if not api_key:
            return False

        model_uid = model['uid']
        model_name = model['name']
        author = model['user']['username']

        self.log(f"\n{'='*60}")
        self.log(f"Downloading: {model_name}")
        self.log(f"Author: {author}")
        self.log(f"License: {model.get('license', {}).get('label', 'Unknown')}")
        self.log(f"{'='*60}")

        try:
            # Get download URL
            download_url = f"https://api.sketchfab.com/v3/models/{model_uid}/download"
            headers = {"Authorization": f"Token {api_key}"}

            response = requests.get(download_url, headers=headers, timeout=30)
            response.raise_for_status()

            download_data = response.json()

            # Get GLTF format (best for UE5)
            if 'gltf' in download_data:
                file_url = download_data['gltf']['url']
            else:
                self.log("⚠️  GLTF format not available, trying source...")
                if 'source' in download_data:
                    file_url = download_data['source']['url']
                else:
                    self.log("❌ No downloadable format available")
                    self.failed_count += 1
                    return False

            # Create safe filename
            safe_name = "".join(c for c in model_name if c.isalnum() or c in (' ', '-', '_')).rstrip()
            safe_name = safe_name.replace(' ', '_')

            # Create author directory
            author_dir = SKETCHFAB_DIR / author
            author_dir.mkdir(parents=True, exist_ok=True)

            dest_path = author_dir / f"{safe_name}.zip"

            # Check if already downloaded
            if dest_path.exists():
                self.log(f"⏭️  Skipping (already exists)")
                return True

            # Download the file
            if self.download_file(file_url, dest_path):
                # Save metadata
                metadata_file = author_dir / f"{safe_name}_metadata.json"
                with open(metadata_file, 'w', encoding='utf-8') as f:
                    json.dump({
                        'name': model_name,
                        'author': author,
                        'license': model.get('license', {}).get('label', 'Unknown'),
                        'url': f"https://sketchfab.com/3d-models/{model_uid}",
                        'attribution': f"{model_name} by {author} (https://sketchfab.com/3d-models/{model_uid})"
                    }, f, indent=2)

                self.log(f"✅ Saved metadata to {metadata_file.name}")
                self.downloaded_count += 1
                return True
            else:
                self.failed_count += 1
                return False

        except Exception as e:
            self.log(f"❌ Failed to download model: {e}")
            self.failed_count += 1
            return False

    def download_all_sketchfab(self):
        """Download all configured Sketchfab models"""
        if not self.config.get('sketchfab', {}).get('enabled', False):
            self.log("\n⏭️  Sketchfab downloads disabled in config")
            return

        api_key = self.config.get('api_keys', {}).get('sketchfab', '')
        if not api_key:
            self.log("\n⚠️  Sketchfab API key not configured. Get one at: https://sketchfab.com/settings/password")
            return

        self.log("\n" + "="*60)
        self.log("STARTING SKETCHFAB DOWNLOAD")
        self.log("="*60)

        SKETCHFAB_DIR.mkdir(parents=True, exist_ok=True)

        searches = self.config.get('sketchfab', {}).get('searches', [])
        self.log(f"Queries to run: {len(searches)}")

        for search in searches:
            query = search.get('query', '')
            license_type = search.get('license', 'cc0')
            count = search.get('count', 5)

            models = self.search_sketchfab_models(query, license_type, count)

            for model in models:
                self.download_sketchfab_model(model)
                time.sleep(1)  # Be nice to the API

            time.sleep(2)  # Pause between searches

    def search_freesound(self, query, count=5):
        """Search for sounds on Freesound"""
        api_key = self.config.get('api_keys', {}).get('freesound', '')
        if not api_key:
            self.log("⚠️  Freesound API key not configured in asset_config.json")
            return []

        try:
            url = "https://freesound.org/apiv2/search/text/"
            params = {
                "query": query,
                "token": api_key,
                "filter": "license:\"Creative Commons 0\"",
                "fields": "id,name,username,license,previews,download",
                "page_size": count
            }

            self.log(f"🔍 Searching Freesound for: {query}")
            response = requests.get(url, params=params, timeout=30)
            response.raise_for_status()

            results = response.json().get('results', [])
            self.log(f"✅ Found {len(results)} sounds")

            return results

        except Exception as e:
            self.log(f"❌ Failed to search Freesound: {e}")
            return []

    def download_freesound(self, sound):
        """Download a single sound from Freesound"""
        api_key = self.config.get('api_keys', {}).get('freesound', '')
        if not api_key:
            return False

        sound_id = sound['id']
        sound_name = sound['name']
        username = sound['username']

        self.log(f"\n{'='*60}")
        self.log(f"Downloading: {sound_name}")
        self.log(f"Author: {username}")
        self.log(f"{'='*60}")

        try:
            # Get download URL
            download_url = f"https://freesound.org/apiv2/sounds/{sound_id}/download/"
            headers = {"Authorization": f"Token {api_key}"}

            # Create safe filename
            safe_name = "".join(c for c in sound_name if c.isalnum() or c in (' ', '-', '_')).rstrip()
            safe_name = safe_name.replace(' ', '_')

            # Determine file extension from previews
            file_ext = ".wav"  # Default
            if 'download' in sound:
                file_ext = Path(sound['download']).suffix or ".wav"

            # Create category directory
            category_dir = FREESOUND_DIR / username
            category_dir.mkdir(parents=True, exist_ok=True)

            dest_path = category_dir / f"{safe_name}{file_ext}"

            # Check if already downloaded
            if dest_path.exists():
                self.log(f"⏭️  Skipping (already exists)")
                return True

            # Download with redirect following
            response = requests.get(download_url, headers=headers, allow_redirects=True, timeout=60)
            response.raise_for_status()

            # Save file
            with open(dest_path, 'wb') as f:
                f.write(response.content)

            file_size = dest_path.stat().st_size / 1024 / 1024
            self.log(f"✅ Downloaded {dest_path.name} ({file_size:.1f} MB)")

            # Save metadata
            metadata_file = category_dir / f"{safe_name}_metadata.json"
            with open(metadata_file, 'w', encoding='utf-8') as f:
                json.dump({
                    'name': sound_name,
                    'author': username,
                    'license': sound.get('license', 'CC0'),
                    'url': f"https://freesound.org/people/{username}/sounds/{sound_id}/",
                    'attribution': f"{sound_name} by {username} (https://freesound.org/people/{username}/sounds/{sound_id}/)"
                }, f, indent=2)

            self.downloaded_count += 1
            return True

        except Exception as e:
            self.log(f"❌ Failed to download sound: {e}")
            self.failed_count += 1
            return False

    def download_all_freesound(self):
        """Download all configured Freesound audio"""
        if not self.config.get('freesound', {}).get('enabled', False):
            self.log("\n⏭️  Freesound downloads disabled in config")
            return

        api_key = self.config.get('api_keys', {}).get('freesound', '')
        if not api_key:
            self.log("\n⚠️  Freesound API key not configured. Get one at: https://freesound.org/apiv2/apply/")
            return

        self.log("\n" + "="*60)
        self.log("STARTING FREESOUND DOWNLOAD")
        self.log("="*60)

        FREESOUND_DIR.mkdir(parents=True, exist_ok=True)

        searches = self.config.get('freesound', {}).get('searches', [])
        self.log(f"Queries to run: {len(searches)}")

        for search in searches:
            query = search.get('query', '')
            count = search.get('count', 5)

            sounds = self.search_freesound(query, count)

            for sound in sounds:
                self.download_freesound(sound)
                time.sleep(1)  # Be nice to the API

            time.sleep(2)  # Pause between searches

    def run(self):
        """Main execution"""
        try:
            start_time = time.time()

            self.log("\n" + "="*60)
            self.log("STEEL CATHEDRAL ASSET DOWNLOADER")
            self.log("="*60)

            # Download Poly Haven materials
            if self.config.get('polyhaven', {}).get('enabled', True):
                self.download_all_polyhaven()
                self.download_all_hdris()
                self.update_cpp_material_loader()

            # Download Sketchfab models
            self.download_all_sketchfab()

            # Download Freesound audio
            self.download_all_freesound()

            # Create summary
            elapsed = time.time() - start_time
            self.log(f"\n⏱️  Total time: {elapsed:.1f} seconds")

            self.create_summary()

        except KeyboardInterrupt:
            self.log("\n⚠️  Download interrupted by user")
        except Exception as e:
            self.log(f"\n❌ Fatal error: {e}")
            import traceback
            self.log(traceback.format_exc())
        finally:
            self.log_file.close()

def main():
    """Main entry point with command-line argument parsing"""
    parser = argparse.ArgumentParser(
        description="Steel Cathedral Asset Downloader - Download from Poly Haven, Sketchfab, and Freesound",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  py download_assets.py                          # Normal download mode (all enabled sources)
  py download_assets.py --list                   # List all available Poly Haven textures
  py download_assets.py --list --type hdris      # List all available HDRIs
  py download_assets.py --search metal           # Search Poly Haven for materials with 'metal'
  py download_assets.py --similar scratched_metal # Find similar names
  py download_assets.py --suggest                # Suggest alternatives for missing materials
  py download_assets.py --search-sketchfab "reactor core" # Search Sketchfab
  py download_assets.py --search-freesound "hydraulic hiss" # Search Freesound
        """
    )

    parser.add_argument('--list', action='store_true',
                        help='List all available Poly Haven assets')
    parser.add_argument('--search', type=str, metavar='KEYWORD',
                        help='Search Poly Haven for assets matching keyword')
    parser.add_argument('--similar', type=str, metavar='NAME',
                        help='Find Poly Haven assets with similar names')
    parser.add_argument('--suggest', action='store_true',
                        help='Suggest alternatives for missing materials')
    parser.add_argument('--type', type=str, default='textures',
                        choices=['textures', 'hdris'],
                        help='Poly Haven asset type to search (default: textures)')
    parser.add_argument('--count', type=int, default=5, metavar='N',
                        help='Number of similar results to show (default: 5)')
    parser.add_argument('--search-sketchfab', type=str, metavar='QUERY',
                        help='Search Sketchfab for models (requires API key in config)')
    parser.add_argument('--search-freesound', type=str, metavar='QUERY',
                        help='Search Freesound for audio (requires API key in config)')
    parser.add_argument('--license', type=str, default='cc0',
                        choices=['cc0', 'cc-by', 'cc-by-sa'],
                        help='License filter for Sketchfab (default: cc0)')

    args = parser.parse_args()

    downloader = AssetDownloader()

    # Handle different modes
    if args.list:
        downloader.list_available_assets(args.type)
    elif args.search:
        downloader.search_assets(args.search, args.type)
    elif args.similar:
        downloader.find_similar(args.similar, args.type, args.count)
    elif args.suggest:
        downloader.suggest_alternatives()
    elif args.search_sketchfab:
        models = downloader.search_sketchfab_models(args.search_sketchfab, args.license, args.count)
        downloader.log(f"\nTo download these models, add them to asset_config.json")
    elif args.search_freesound:
        sounds = downloader.search_freesound(args.search_freesound, args.count)
        downloader.log(f"\nTo download these sounds, add them to asset_config.json")
    else:
        # Default: run normal download
        downloader.run()

    downloader.log_file.close()

if __name__ == "__main__":
    main()
