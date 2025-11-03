#!/usr/bin/env python3
"""
FULLY AUTOMATED TEST PIPELINE
Builds, launches Unreal, captures screenshots, analyzes, reports
NO USER INTERACTION REQUIRED
"""

import subprocess
import time
import sys
from pathlib import Path
from datetime import datetime

# Fix Windows console encoding
if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8')

# Paths
PROJECT_ROOT = Path(__file__).parent.parent
PROJECT_FILE = PROJECT_ROOT / "SteelCathedrals.uproject"
UE_EDITOR = Path(r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe")
UE_CMD = Path(r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
SCREENSHOT_DIR = PROJECT_ROOT / "Saved" / "Screenshots" / "WindowsEditor"
LOG_FILE = PROJECT_ROOT / "Saved" / "Logs" / "AutoTest.log"

def print_header(text):
    """Print a nice header"""
    print()
    print("=" * 80)
    print(text)
    print("=" * 80)
    print()

def build_project():
    """Build the project"""
    print_header("STEP 1: BUILDING PROJECT")

    cmd = [
        "dotnet",
        r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll",
        "SteelCathedralsEditor",
        "Win64",
        "Development",
        f"-Project={PROJECT_FILE}",
        "-WaitMutex"
    ]

    print("Building SteelCathedralsEditor...")
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_ROOT)

    if result.returncode != 0:
        print("❌ Build failed!")
        print(result.stdout)
        print(result.stderr)
        return False

    print("✅ Build succeeded")
    return True

def clean_old_screenshots():
    """Remove old screenshots"""
    if not SCREENSHOT_DIR.exists():
        SCREENSHOT_DIR.mkdir(parents=True, exist_ok=True)
        return

    for old_screenshot in SCREENSHOT_DIR.glob("*.png"):
        old_screenshot.unlink()

    print(f"🧹 Cleaned screenshot directory: {SCREENSHOT_DIR}")

def launch_and_capture():
    """Launch Unreal, spawn everything, take screenshots"""
    print_header("STEP 2: LAUNCHING UNREAL & CAPTURING SCREENSHOTS")

    clean_old_screenshots()

    # Note: Auto-capture is now handled by GameMode when bAutoCaptureScreenshots=true
    # We don't need console commands anymore
    exec_cmds = "stat fps"  # Just show FPS for debugging

    # Build command
    cmd = [
        str(UE_EDITOR),
        str(PROJECT_FILE),
        "-game",  # Game mode (not editor mode)
        "-windowed",
        "-ResX=1920",
        "-ResY=1080",
        "-AutoCapture",  # Enable auto-capture in GameMode
        f"-ExecCmds={exec_cmds}",
        "-Unattended",  # No user interaction
        "-NoSplash",
        "-NoSound",
        f"-Log={LOG_FILE}",
    ]

    print("Launching Unreal Editor...")
    print(f"Log file: {LOG_FILE}")
    print()
    print("⏳ Waiting for screenshot capture (30 seconds)...")

    # Launch in background
    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=PROJECT_ROOT
    )

    # Wait for screenshots to be captured
    # The ScreenshotCapture actor needs about 5 seconds to capture all 7 shots
    wait_time = 30
    start_time = time.time()

    while time.time() - start_time < wait_time:
        elapsed = int(time.time() - start_time)
        remaining = wait_time - elapsed

        # Check if screenshots are appearing
        screenshots = list(SCREENSHOT_DIR.glob("*.png"))

        print(f"\r  [{elapsed}/{wait_time}s] Found {len(screenshots)} screenshot(s)...", end="", flush=True)

        # If we have 7+ screenshots, we're done
        if len(screenshots) >= 7:
            print()
            print("✅ All screenshots captured!")
            break

        time.sleep(1)

    print()

    # Kill the process
    try:
        process.terminate()
        process.wait(timeout=5)
    except:
        process.kill()

    return True

def collect_and_analyze_screenshots():
    """Collect screenshots and prepare for analysis"""
    print_header("STEP 3: COLLECTING SCREENSHOTS")

    if not SCREENSHOT_DIR.exists():
        print("❌ No screenshot directory found!")
        return []

    screenshots = list(SCREENSHOT_DIR.glob("*.png"))

    if not screenshots:
        print("❌ No screenshots captured!")
        print()
        print("Possible issues:")
        print("  - Build failed")
        print("  - Mech didn't spawn")
        print("  - ScreenshotCapture actor didn't spawn")
        print(f"  - Check log: {LOG_FILE}")
        return []

    print(f"📸 Found {len(screenshots)} screenshot(s):")
    print()

    # Copy to project root with timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    copied_screenshots = []

    for screenshot in sorted(screenshots):
        size_mb = screenshot.stat().st_size / (1024 * 1024)
        dest_name = f"test_{screenshot.stem}_{timestamp}.png"
        dest_path = PROJECT_ROOT / dest_name

        # Copy
        import shutil
        shutil.copy2(screenshot, dest_path)

        print(f"  ✅ {dest_name} ({size_mb:.2f} MB)")
        copied_screenshots.append(dest_path)

    print()
    print(f"📁 Copied {len(copied_screenshots)} screenshot(s) to project root")

    return copied_screenshots

def report_results(screenshots):
    """Generate report"""
    print_header("STEP 4: TEST RESULTS")

    if not screenshots:
        print("❌ TEST FAILED - No screenshots captured")
        print()
        print("Review the log file for errors:")
        print(f"  {LOG_FILE}")
        return False

    print("✅ TEST SUCCEEDED")
    print()
    print(f"Captured {len(screenshots)} screenshots for analysis:")
    print()

    for screenshot in screenshots:
        print(f"  • {screenshot.name}")

    print()
    print("🤖 AI ANALYSIS:")
    print("   Screenshots are ready for visual inspection.")
    print("   The AI assistant can now:")
    print("     - Read these image files")
    print("     - Identify geometry issues")
    print("     - Spot clipping problems")
    print("     - Validate spawn positions")
    print("     - Report findings automatically")
    print()
    print("Next steps:")
    print("  1. AI will analyze the screenshots")
    print("  2. AI will report any issues found")
    print("  3. AI will propose fixes")
    print("  4. Repeat until tests pass")

    return True

def main():
    """Run the full automated test pipeline"""
    print()
    print("╔════════════════════════════════════════════════════════════════════════════╗")
    print("║                   AUTOMATED SCREENSHOT TEST PIPELINE                       ║")
    print("║                         No User Input Required                             ║")
    print("╚════════════════════════════════════════════════════════════════════════════╝")

    start_time = time.time()

    # Step 1: Build
    if not build_project():
        print()
        print("❌ PIPELINE FAILED AT BUILD STEP")
        return 1

    # Step 2: Launch and capture
    if not launch_and_capture():
        print()
        print("❌ PIPELINE FAILED AT CAPTURE STEP")
        return 1

    # Step 3: Collect screenshots
    screenshots = collect_and_analyze_screenshots()

    # Step 4: Report
    success = report_results(screenshots)

    elapsed = time.time() - start_time

    print()
    print("=" * 80)
    print(f"Pipeline completed in {elapsed:.1f} seconds")
    print("=" * 80)
    print()

    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
