#!/usr/bin/env python3
"""
Automated test runner for Steel Cathedrals
Runs Unreal automation tests without opening the editor UI
"""

import subprocess
import sys
import os
from pathlib import Path

# Fix Windows console encoding
if sys.platform == 'win32':
    sys.stdout.reconfigure(encoding='utf-8')

# Paths
PROJECT_ROOT = Path(__file__).parent.parent
PROJECT_FILE = PROJECT_ROOT / "SteelCathedrals.uproject"
UE_EDITOR = Path(r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
LOG_FILE = PROJECT_ROOT / "Saved" / "Logs" / "TestRun.log"

def run_unreal_tests():
    """Run Unreal automation tests in headless mode"""

    print("=" * 80)
    print("STEEL CATHEDRALS - AUTOMATED TEST RUNNER")
    print("=" * 80)
    print()

    if not PROJECT_FILE.exists():
        print(f"❌ ERROR: Project file not found: {PROJECT_FILE}")
        return False

    if not UE_EDITOR.exists():
        print(f"❌ ERROR: Unreal Editor not found: {UE_EDITOR}")
        return False

    print(f"✅ Project: {PROJECT_FILE}")
    print(f"✅ Editor: {UE_EDITOR}")
    print()
    print("Running tests...")
    print("-" * 80)

    # Run automation tests
    cmd = [
        str(UE_EDITOR),
        str(PROJECT_FILE),
        "-ExecCmds=Automation RunTests SteelCathedrals.Interior",
        "-TestExit=Automation Test Queue Empty",
        "-NoSound",
        "-NullRHI",  # No rendering
        "-Unattended",
        "-NoSplash",
        "-Log",
        f"-Log={LOG_FILE}"
    ]

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=120  # 2 minute timeout
        )

        print()
        print("-" * 80)
        print("TEST OUTPUT:")
        print("-" * 80)

        # Parse output for test results
        output = result.stdout + result.stderr

        # Look for test results
        lines = output.split('\n')
        test_count = 0
        passed_count = 0
        failed_count = 0

        for line in lines:
            if 'LogAutomationCommandLine' in line or 'LogAutomation' in line:
                print(line)

                if 'Test Completed' in line:
                    test_count += 1
                    if 'Success' in line:
                        passed_count += 1
                    else:
                        failed_count += 1

        print()
        print("=" * 80)
        print("TEST SUMMARY")
        print("=" * 80)
        print(f"Total Tests: {test_count}")
        print(f"✅ Passed: {passed_count}")
        print(f"❌ Failed: {failed_count}")
        print()

        if LOG_FILE.exists():
            print(f"Full log: {LOG_FILE}")

        return failed_count == 0

    except subprocess.TimeoutExpired:
        print()
        print("❌ ERROR: Test run timed out!")
        return False
    except Exception as e:
        print()
        print(f"❌ ERROR: {e}")
        return False

def quick_validation():
    """Quick validation checks without launching Unreal"""

    print()
    print("=" * 80)
    print("QUICK VALIDATION (No Unreal Launch)")
    print("=" * 80)
    print()

    errors = []

    # Check spawn locations
    print("Checking spawn locations...")

    interior_offset = (200.0, 0.0, 2500.0)
    cockpit_min = (-300.0, -250.0, 0.0)
    cockpit_max = (300.0, 250.0, 250.0)
    wall_thickness = 20.0
    char_radius = 40.0

    # Spawn locations are now calculated from station positions
    # Station positions: Pilot(-200,-150), Gunner(-200,150), Center(0,0)
    # Crew spawn BEHIND stations (+80cm in X direction) at standing height (100cm)
    spawn_locations = [
        ("Player 1", (-200.0 + 80.0, -150.0, 100.0)),  # Behind Pilot station
        ("Player 2", (-200.0 + 80.0, 150.0, 100.0)),    # Behind Gunner station
        ("Player 3", (0.0, 0.0, 100.0))                  # Center
    ]

    for name, spawn in spawn_locations:
        # Convert to local position (relative to interior)
        local_x = spawn[0]
        local_y = spawn[1]
        local_z = spawn[2]

        # Check walls
        safe_min_x = cockpit_min[0] + wall_thickness + char_radius
        safe_max_x = cockpit_max[0] - wall_thickness - char_radius
        safe_min_y = cockpit_min[1] + wall_thickness + char_radius
        safe_max_y = cockpit_max[1] - wall_thickness - char_radius

        is_valid = True

        if local_x < safe_min_x:
            errors.append(f"❌ {name} too close to LEFT WALL: x={local_x:.1f} (min safe: {safe_min_x:.1f})")
            is_valid = False

        if local_x > safe_max_x:
            errors.append(f"❌ {name} too close to RIGHT WALL: x={local_x:.1f} (max safe: {safe_max_x:.1f})")
            is_valid = False

        if local_y < safe_min_y:
            errors.append(f"❌ {name} too close to FRONT WALL: y={local_y:.1f} (min safe: {safe_min_y:.1f})")
            is_valid = False

        if local_y > safe_max_y:
            errors.append(f"❌ {name} too close to BACK WALL: y={local_y:.1f} (max safe: {safe_max_y:.1f})")
            is_valid = False

        if local_z < 0:
            errors.append(f"❌ {name} BELOW FLOOR: z={local_z:.1f}")
            is_valid = False

        if local_z > cockpit_max[2]:
            errors.append(f"❌ {name} ABOVE CEILING: z={local_z:.1f} (max: {cockpit_max[2]:.1f})")
            is_valid = False

        if is_valid:
            print(f"  ✅ {name} at ({local_x:.1f}, {local_y:.1f}, {local_z:.1f}) - OK")

    # Check station positions
    print()
    print("Checking station positions...")

    stations = [
        ("Pilot", (-200, -150, 0)),
        ("Gunner", (-200, 150, 0)),
        ("Technician", (150, -150, 0)),
        ("Navigation", (150, 150, 0))
    ]

    for name, pos in stations:
        if (cockpit_min[0] <= pos[0] <= cockpit_max[0] and
            cockpit_min[1] <= pos[1] <= cockpit_max[1]):
            print(f"  ✅ {name} station at ({pos[0]}, {pos[1]}, {pos[2]}) - OK")
        else:
            errors.append(f"❌ {name} station OUTSIDE cockpit at ({pos[0]}, {pos[1]}, {pos[2]})")

    print()
    print("=" * 80)

    if errors:
        print("❌ VALIDATION FAILED")
        print("=" * 80)
        for error in errors:
            print(error)
        return False
    else:
        print("✅ ALL VALIDATIONS PASSED")
        print("=" * 80)
        return True

def check_screenshots():
    """Check if screenshots exist and display them"""
    screenshot_dir = PROJECT_ROOT / "Saved" / "Screenshots" / "WindowsEditor"

    if not screenshot_dir.exists():
        return []

    screenshots = list(screenshot_dir.glob("*.png"))
    screenshots.sort(key=lambda p: p.stat().st_mtime, reverse=True)  # Most recent first

    return screenshots[:10]  # Return up to 10 most recent

def print_screenshot_instructions():
    """Print instructions for taking automated screenshots"""
    print()
    print("=" * 80)
    print("AUTOMATED SCREENSHOT CAPTURE")
    print("=" * 80)
    print()
    print("To capture multi-angle screenshots for analysis:")
    print()
    print("1. Open Unreal Editor")
    print("2. Press Play (PIE)")
    print("3. Open console (~ key) and type:")
    print("   SpawnActor ScreenshotCapture")
    print("4. Then type:")
    print("   ke * StartCaptureSequence")
    print()
    print("This will automatically capture 7 screenshots from different angles:")
    print("  - Front view")
    print("  - Left/Right sides")
    print("  - Top-down")
    print("  - Diagonal overview")
    print("  - Interior cockpit")
    print("  - Close-up head")
    print()
    print(f"Screenshots save to: {PROJECT_ROOT / 'Saved' / 'Screenshots' / 'WindowsEditor'}")
    print()
    print("=" * 80)

if __name__ == "__main__":
    # Run quick validation first
    quick_valid = quick_validation()

    # Check for existing screenshots
    screenshots = check_screenshots()
    if screenshots:
        print()
        print(f"Found {len(screenshots)} recent screenshot(s):")
        for i, screenshot in enumerate(screenshots[:5], 1):
            age_seconds = (Path.ctime(screenshot) - Path.ctime(screenshot))
            print(f"  {i}. {screenshot.name}")

    # Ask what to do
    print()
    if not quick_valid:
        print("⚠️  Quick validation found issues!")

    print()
    print("Options:")
    print("  [V] Run quick validation again")
    print("  [S] Show screenshot instructions")
    print("  [T] Run full Unreal automation tests (slow)")
    print("  [Q] Quit")
    print()

    response = input("Choice [V/s/t/q]: ").strip().lower() or 'v'

    if response == 's':
        print_screenshot_instructions()
    elif response == 't':
        success = run_unreal_tests()
        sys.exit(0 if success else 1)
    elif response == 'v':
        sys.exit(0 if quick_valid else 1)
    else:
        print("Exiting.")
        sys.exit(0 if quick_valid else 1)
