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
LOG_FILE = PROJECT_ROOT / "asset_download.log"

# Resolution to download (2k is good balance, 1k for HDRIs)
RESOLUTION = "2k"
HDRI_RESOLUTION = "1k"

# Materials to download from Poly Haven
POLYHAVEN_MATERIALS = [
    # Metals (mech exterior)
    "scratched_metal",
    "painted_metal_01",
    "corrugated_iron",
    "brushed_metal",
    "carbon_fiber",
    "steel_plate",
    "aluminum_plate",
    "galvanized_metal",
    "metal_plate",
    "rusty_metal_02",
    "metal_grill",
    "weathered_metal",
    "titanium_scuffed",
    "diamond_plate",

    # Industrial (interior panels)
    "industrial_panel",
    "metal_grid_floor",
    "rough_metal_plates",
    "industrial_flooring",
    "grated_metal_floor",

    # Concrete/Structure (terrain, ruins)
    "worn_concrete_01",
    "concrete_wall_001",
    "concrete_floor_001",
    "rough_concrete_01",
    "concrete_panels_01",
    "damaged_concrete",
    "concrete_blocks",

    # Rubber/Cables (interior details)
    "rubber_tiles_01",
    "cable_bundle",

    # Dirt/Grime (weathering)
    "dirt_01",
    "grime_01",
]

# HDRIs for lighting (smaller 1k for performance)
POLYHAVEN_HDRIS = [
    "industrial_sunset_02",
    "wasteland_clouds",
    "venice_sunset",
    "satara_night",
    "abandoned_hall",
]

class AssetDownloader:
    def __init__(self):
        self.log_file = open(LOG_FILE, 'a', encoding='utf-8', buffering=1)
        self.downloaded_count = 0
        self.failed_count = 0

    def log(self, message):
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        log_msg = f"[{timestamp}] {message}"
        print(log_msg)
        self.log_file.write(log_msg + "\n")

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

    def run(self):
        """Main execution"""
        try:
            start_time = time.time()

            self.log("\n" + "="*60)
            self.log("STEEL CATHEDRAL ASSET DOWNLOADER")
            self.log("="*60)

            # Download Poly Haven materials
            self.download_all_polyhaven()

            # Download Poly Haven HDRIs
            self.download_all_hdris()

            # Generate C++ code
            self.update_cpp_material_loader()

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
        description="Steel Cathedral Asset Downloader - Download and discover Poly Haven assets",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  py download_assets.py                          # Normal download mode
  py download_assets.py --list                   # List all available textures
  py download_assets.py --list --type hdris      # List all available HDRIs
  py download_assets.py --search metal           # Search for materials with 'metal'
  py download_assets.py --similar scratched_metal # Find similar names
  py download_assets.py --suggest                # Suggest alternatives for missing materials
        """
    )

    parser.add_argument('--list', action='store_true',
                        help='List all available assets')
    parser.add_argument('--search', type=str, metavar='KEYWORD',
                        help='Search for assets matching keyword')
    parser.add_argument('--similar', type=str, metavar='NAME',
                        help='Find assets with similar names')
    parser.add_argument('--suggest', action='store_true',
                        help='Suggest alternatives for missing materials')
    parser.add_argument('--type', type=str, default='textures',
                        choices=['textures', 'hdris'],
                        help='Asset type to search (default: textures)')
    parser.add_argument('--count', type=int, default=5, metavar='N',
                        help='Number of similar results to show (default: 5)')

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
    else:
        # Default: run normal download
        downloader.run()

    downloader.log_file.close()

if __name__ == "__main__":
    main()
