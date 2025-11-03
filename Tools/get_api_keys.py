#!/usr/bin/env python3
"""
Interactive script to help get API keys for Sketchfab and Freesound
"""

import webbrowser
import json
from pathlib import Path

CONFIG_FILE = Path(__file__).parent / "asset_config.json"

def load_config():
    """Load existing config"""
    if CONFIG_FILE.exists():
        with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    return {}

def save_config(config):
    """Save config back to file"""
    with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
        json.dump(config, f, indent=2)

def main():
    print("\n" + "="*60)
    print("API KEY SETUP HELPER")
    print("="*60)

    config = load_config()
    if 'api_keys' not in config:
        config['api_keys'] = {}

    # Sketchfab
    print("\n[SKETCHFAB] 3D Models")
    print("-" * 60)
    print("Opening Sketchfab API settings page...")
    print("1. Log in (or create free account)")
    print("2. Scroll down to 'API & Downloads' section")
    print("3. Copy your API Token")

    webbrowser.open("https://sketchfab.com/settings/password")

    sketchfab_key = input("\nPaste your Sketchfab API token (or press Enter to skip): ").strip()
    if sketchfab_key:
        config['api_keys']['sketchfab'] = sketchfab_key
        if 'sketchfab' not in config:
            config['sketchfab'] = {'enabled': False}
        print("[OK] Sketchfab API key saved!")
    else:
        print("[SKIP] Skipped Sketchfab")

    # Freesound
    print("\n[FREESOUND] Audio/SFX")
    print("-" * 60)
    print("Opening Freesound API application page...")
    print("1. Log in (or create free account)")
    print("2. Fill out the simple form (instant approval)")
    print("3. Copy your API Key from the confirmation page")

    webbrowser.open("https://freesound.org/apiv2/apply/")

    freesound_key = input("\nPaste your Freesound API key (or press Enter to skip): ").strip()
    if freesound_key:
        config['api_keys']['freesound'] = freesound_key
        if 'freesound' not in config:
            config['freesound'] = {'enabled': False}
        print("[OK] Freesound API key saved!")
    else:
        print("[SKIP] Skipped Freesound")

    # Save config
    save_config(config)

    print("\n" + "="*60)
    print("CONFIGURATION SAVED")
    print("="*60)
    print(f"Config file: {CONFIG_FILE}")

    if sketchfab_key or freesound_key:
        print("\n[NEXT STEPS]")
        print("1. Edit asset_config.json")

        if sketchfab_key:
            print("2. Set sketchfab.enabled = true")
        if freesound_key:
            print("3. Set freesound.enabled = true")

        print("4. Run: py download_assets.py")
        print("\nOr test search:")
        if sketchfab_key:
            print('   py download_assets.py --search-sketchfab "reactor core"')
        if freesound_key:
            print('   py download_assets.py --search-freesound "hydraulic hiss"')
    else:
        print("\n[WARNING] No API keys configured.")
        print("You can still use Poly Haven (no API key needed)!")
        print("Run this script again anytime to add keys.")

    print("\n")

if __name__ == "__main__":
    main()
