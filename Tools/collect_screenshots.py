#!/usr/bin/env python3
"""
Collect screenshots from Unreal and copy them to project root for analysis
"""

import shutil
from pathlib import Path
from datetime import datetime

PROJECT_ROOT = Path(__file__).parent.parent
SCREENSHOT_SOURCE = PROJECT_ROOT / "Saved" / "Screenshots" / "WindowsEditor"
SCREENSHOT_DEST = PROJECT_ROOT

def collect_screenshots():
    """Copy recent screenshots to project root with timestamp"""

    if not SCREENSHOT_SOURCE.exists():
        print(f"❌ No screenshots found at: {SCREENSHOT_SOURCE}")
        print()
        print("Make sure you:")
        print("  1. Opened Unreal Editor")
        print("  2. Pressed Play")
        print("  3. Spawned ScreenshotCapture actor")
        print("  4. Called StartCaptureSequence")
        return

    screenshots = list(SCREENSHOT_SOURCE.glob("*.png"))

    if not screenshots:
        print(f"❌ No PNG files found in: {SCREENSHOT_SOURCE}")
        return

    # Sort by modification time, most recent first
    screenshots.sort(key=lambda p: p.stat().st_mtime, reverse=True)

    print()
    print("=" * 80)
    print("SCREENSHOT COLLECTION")
    print("=" * 80)
    print()
    print(f"Found {len(screenshots)} screenshot(s)")
    print()

    # Copy up to 10 most recent screenshots
    copied = 0
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    for screenshot in screenshots[:10]:
        # Create descriptive name
        dest_name = f"screenshot_{screenshot.stem}_{timestamp}.png"
        dest_path = SCREENSHOT_DEST / dest_name

        try:
            shutil.copy2(screenshot, dest_path)
            size_mb = dest_path.stat().st_size / (1024 * 1024)
            print(f"✅ Copied: {dest_name} ({size_mb:.2f} MB)")
            copied += 1
        except Exception as e:
            print(f"❌ Failed to copy {screenshot.name}: {e}")

    print()
    print(f"📸 Copied {copied} screenshot(s) to project root")
    print()
    print("These screenshots can now be analyzed by the AI assistant.")
    print()

if __name__ == "__main__":
    collect_screenshots()
