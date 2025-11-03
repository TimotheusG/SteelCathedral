#!/usr/bin/env python3
"""
Automated screenshot test for Steel Cathedrals
Takes screenshots from multiple angles and saves them for analysis
"""

import subprocess
import time
import os
from pathlib import Path
from datetime import datetime

# Paths
PROJECT_ROOT = Path(__file__).parent.parent
PROJECT_FILE = PROJECT_ROOT / "SteelCathedrals.uproject"
UE_EDITOR = Path(r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
SCREENSHOT_DIR = PROJECT_ROOT / "Saved" / "Screenshots" / "WindowsEditor"

# Camera positions for screenshots (relative to mech at origin)
CAMERA_POSITIONS = [
    {
        "name": "front_view",
        "position": (3000, 0, 2000),  # 30m in front, looking at torso
        "rotation": (0, 180, 0),
        "description": "Front view of mech"
    },
    {
        "name": "side_left",
        "position": (0, -3000, 2000),  # 30m to the left
        "rotation": (0, 90, 0),
        "description": "Left side view"
    },
    {
        "name": "side_right",
        "position": (0, 3000, 2000),  # 30m to the right
        "rotation": (0, -90, 0),
        "description": "Right side view"
    },
    {
        "name": "top_down",
        "position": (0, 0, 6000),  # 60m above
        "rotation": (-90, 0, 0),
        "description": "Top-down view"
    },
    {
        "name": "interior_cockpit",
        "position": (200, 0, 2600),  # Inside cockpit
        "rotation": (0, 0, 0),
        "description": "Interior cockpit view"
    },
    {
        "name": "diagonal_overview",
        "position": (4000, 4000, 4000),  # Isometric-ish
        "rotation": (-30, -135, 0),
        "description": "Diagonal overview"
    }
]

def create_screenshot_commands():
    """Generate Unreal console commands for taking screenshots"""
    commands = []

    # Spawn mech at origin
    commands.append("SpawnActor Mech 0 0 0")

    # Wait a bit for everything to spawn
    commands.append("Delay 2.0")

    # Take screenshots from each angle
    for cam_info in CAMERA_POSITIONS:
        pos = cam_info["position"]
        rot = cam_info["rotation"]
        name = cam_info["name"]

        # Set camera position and rotation
        # Note: Unreal console commands for camera control
        commands.append(f"Camera SetLocation {pos[0]} {pos[1]} {pos[2]}")
        commands.append(f"Camera SetRotation {rot[0]} {rot[1]} {rot[2]}")

        # Take high-res screenshot
        commands.append(f"HighResShot 1920x1080 filename={name}")

        # Wait a moment between screenshots
        commands.append("Delay 0.5")

    return commands

def run_screenshot_session():
    """Launch Unreal and take automated screenshots"""

    print("=" * 80)
    print("AUTOMATED SCREENSHOT TEST")
    print("=" * 80)
    print()

    if not PROJECT_FILE.exists():
        print(f"❌ ERROR: Project file not found: {PROJECT_FILE}")
        return False

    if not UE_EDITOR.exists():
        print(f"❌ ERROR: Unreal Editor not found: {UE_EDITOR}")
        return False

    # Create screenshot directory if needed
    SCREENSHOT_DIR.mkdir(parents=True, exist_ok=True)

    # Clear old screenshots
    for old_screenshot in SCREENSHOT_DIR.glob("*.png"):
        try:
            old_screenshot.unlink()
            print(f"  Removed old screenshot: {old_screenshot.name}")
        except Exception as e:
            print(f"  Warning: Could not remove {old_screenshot.name}: {e}")

    print()
    print("📸 Starting screenshot session...")
    print(f"   Screenshots will be saved to: {SCREENSHOT_DIR}")
    print()

    # Build exec commands string
    exec_commands = []

    # Commands to take screenshots
    for cam_info in CAMERA_POSITIONS:
        # Use simpler approach - just take screenshots with delay
        exec_commands.append(f"HighResShot 1920x1080 filename={cam_info['name']}")

    exec_string = ";".join(exec_commands)

    # Launch Unreal in game mode (not editor) for faster startup
    cmd = [
        str(UE_EDITOR),
        str(PROJECT_FILE),
        "-game",  # Game mode (faster than editor)
        "-windowed",  # Windowed mode
        "-ResX=1920",
        "-ResY=1080",
        f"-ExecCmds={exec_string}",
        "-Benchmark",  # Benchmark mode for automation
        "-NoSound",
        "-NoSplash",
        "-Unattended",
        f"-Log={PROJECT_ROOT / 'Saved' / 'Logs' / 'ScreenshotTest.log'}",
        "-seconds=30"  # Auto-exit after 30 seconds
    ]

    print("🚀 Launching Unreal Editor...")
    print()

    try:
        # Run with timeout
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=60  # 1 minute timeout
        )

        print("✅ Screenshot session completed")
        print()

        # List captured screenshots
        screenshots = list(SCREENSHOT_DIR.glob("*.png"))
        if screenshots:
            print(f"📸 Captured {len(screenshots)} screenshot(s):")
            for screenshot in sorted(screenshots):
                size_mb = screenshot.stat().st_size / (1024 * 1024)
                print(f"   - {screenshot.name} ({size_mb:.2f} MB)")
        else:
            print("⚠️  No screenshots found!")
            print("   Check the log file for errors:")
            print(f"   {PROJECT_ROOT / 'Saved' / 'Logs' / 'ScreenshotTest.log'}")

        return len(screenshots) > 0

    except subprocess.TimeoutExpired:
        print("⚠️  Screenshot session timed out")
        return False
    except Exception as e:
        print(f"❌ ERROR: {e}")
        return False

def create_simple_batch_script():
    """Create a simple batch script that the user can run manually"""

    batch_file = PROJECT_ROOT / "Tools" / "take_screenshots.bat"

    batch_content = f'''@echo off
echo Taking automated screenshots...
echo.

REM Launch Unreal and take screenshot from player spawn
"{UE_EDITOR}" "{PROJECT_FILE}" -game -windowed -ResX=1920 -ResY=1080 -ExecCmds="HighResShot 1920x1080" -Benchmark -NoSound -NoSplash -seconds=10

echo.
echo Screenshots saved to: {SCREENSHOT_DIR}
echo.
pause
'''

    batch_file.write_text(batch_content)
    print(f"✅ Created batch script: {batch_file}")
    print(f"   Run this manually to take screenshots from player's view")

    return batch_file

if __name__ == "__main__":
    print()
    print("This script automates screenshot capture from multiple angles.")
    print("However, Unreal's automation framework has limitations.")
    print()
    print("RECOMMENDED APPROACH:")
    print("=" * 80)
    print("1. Open Unreal Editor manually")
    print("2. Press Play (PIE)")
    print("3. Press F9 to take a screenshot from current view")
    print("4. Use WASD to move around and take more screenshots")
    print("5. Screenshots save to:", SCREENSHOT_DIR)
    print()
    print("Or, I can create a batch script to launch in game mode...")
    print()

    choice = input("Create simple screenshot batch script? [y/N]: ").strip().lower()

    if choice == 'y':
        batch_file = create_simple_batch_script()
        print()
        print(f"Run: {batch_file}")
    else:
        print("Skipping batch script creation.")
        print()
        print("Manual screenshot instructions:")
        print("  1. Open editor")
        print("  2. Click Play")
        print("  3. Press F9 for screenshot")
        print(f"  4. Find screenshots in: {SCREENSHOT_DIR}")
