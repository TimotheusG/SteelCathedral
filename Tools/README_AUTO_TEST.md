# Fully Automated Testing - ZERO USER INPUT

## The Problem This Solves

**Before:**
- User: "It's broken"
- AI: "What's broken?"
- User: "The geometry"
- AI: "Which geometry?"
- User: "I don't know, some boxes?"
- *20 messages later...*

**Now:**
```bash
python Tools/auto_test.py
```

AI can SEE the actual geometry and fix issues in 1 iteration instead of 20.

---

## What It Does

**Fully automated pipeline:**

1. ✅ **Builds** the project (compiles C++)
2. ✅ **Launches** Unreal in game mode with `-AutoCapture` flag
3. ✅ **Auto-spawns** ScreenshotCapture actor (no console commands needed)
4. ✅ **Captures** 7 screenshots from different angles automatically
5. ✅ **Collects** screenshots to project root with timestamps
6. ✅ **Reports** findings

**Total time:** ~35-40 seconds
**User interaction:** ZERO

---

## Usage

### For AI Assistant (Me!)

When I want to see what's actually rendering:

```bash
python Tools/auto_test.py
```

Then I can read the screenshot files and analyze them.

### For You (Optional)

You don't need to do anything. But if you want to run it manually:

```bash
cd "C:\Users\timge\Documents\Unreal Projects\MechInterior"
python Tools/auto_test.py
```

---

## How It Works

### 1. Command Line Flag
```cpp
// In GameMode constructor
if (FParse::Param(FCommandLine::Get(), TEXT("AutoCapture")))
{
    bAutoCaptureScreenshots = true;
}
```

### 2. Auto-Spawn in StartPlay
```cpp
if (bAutoCaptureScreenshots)
{
    AScreenshotCapture* Actor = SpawnActor(...);

    // Wait 3 seconds for everything to settle
    SetTimer([Actor]() {
        Actor->StartCaptureSequence();
    }, 3.0f);
}
```

### 3. Sequential Capture
- Camera moves to position
- Waits 0.5s
- Takes screenshot
- Repeat for 7 angles

### 4. Python Collects Results
- Monitors screenshot directory
- Copies files to project root
- Ready for AI analysis

---

## What Gets Captured

1. **front_view.png** - 30m in front, looking at torso
2. **left_side.png** - 30m to the left
3. **right_side.png** - 30m to the right
4. **top_down.png** - 60m above (bird's eye)
5. **diagonal_overview.png** - Isometric view
6. **interior_cockpit.png** - Inside the cockpit
7. **closeup_head.png** - Close-up of head/cockpit

---

## Example Output

```
╔════════════════════════════════════════════════════════════════════════════╗
║                   AUTOMATED SCREENSHOT TEST PIPELINE                       ║
║                         No User Input Required                             ║
╚════════════════════════════════════════════════════════════════════════════╝

================================================================================
STEP 1: BUILDING PROJECT
================================================================================

Building SteelCathedralsEditor...
✅ Build succeeded

================================================================================
STEP 2: LAUNCHING UNREAL & CAPTURING SCREENSHOTS
================================================================================

Launching Unreal Editor...
⏳ Waiting for screenshot capture (30 seconds)...
  [7/30s] Found 7 screenshot(s)...
✅ All screenshots captured!

================================================================================
STEP 3: COLLECTING SCREENSHOTS
================================================================================

📸 Found 7 screenshot(s):

  ✅ test_front_view_20251103_180045.png (1.23 MB)
  ✅ test_left_side_20251103_180045.png (1.18 MB)
  ✅ test_right_side_20251103_180045.png (1.20 MB)
  ✅ test_top_down_20251103_180045.png (0.98 MB)
  ✅ test_diagonal_overview_20251103_180045.png (1.45 MB)
  ✅ test_interior_cockpit_20251103_180045.png (1.02 MB)
  ✅ test_closeup_head_20251103_180045.png (1.15 MB)

📁 Copied 7 screenshot(s) to project root

================================================================================
STEP 4: TEST RESULTS
================================================================================

✅ TEST SUCCEEDED

Captured 7 screenshots for analysis:

  • test_front_view_20251103_180045.png
  • test_left_side_20251103_180045.png
  • test_right_side_20251103_180045.png
  • test_top_down_20251103_180045.png
  • test_diagonal_overview_20251103_180045.png
  • test_interior_cockpit_20251103_180045.png
  • test_closeup_head_20251103_180045.png

🤖 AI ANALYSIS:
   Screenshots are ready for visual inspection.

================================================================================
Pipeline completed in 38.2 seconds
================================================================================
```

---

## AI Analysis Workflow

Once screenshots are captured, I can:

1. **Read the image files** using the Read tool
2. **Identify visual issues**:
   - Geometry clipping
   - Missing meshes
   - Wrong colors/materials
   - Spawn position problems
   - Scale issues
   - Interior/exterior misalignment
3. **Propose fixes** based on what I see
4. **Implement fixes**
5. **Run auto_test.py again** to verify
6. **Repeat until perfect**

---

## Troubleshooting

### No screenshots captured
- Check `Saved/Logs/AutoTest.log` for errors
- Make sure mech is spawning (check GameMode logs)
- Try increasing wait time in auto_test.py

### Screenshots are all black
- Lighting issue - check MapSetupActor
- Camera might be inside geometry
- Try adjusting CaptureDistance in ScreenshotCapture

### Build fails
- Close Unreal Editor if open (Live Coding conflict)
- Check for C++ compilation errors
- Try cleaning: `git clean -fdx Intermediate/ Binaries/`

---

## Future Enhancements

- [ ] Add diff comparison between test runs
- [ ] Integrate with CI/CD pipeline
- [ ] Auto-detect visual regressions
- [ ] Generate HTML report with annotated screenshots
- [ ] Add performance metrics capture
- [ ] Support custom camera angles via config file

---

## Why This Matters

This automated visual testing allows rapid iteration without constant back-and-forth:

**Old workflow:**
```
Build → Launch → Test → Close → Describe → Guess → Fix → Repeat (20x)
```

**New workflow:**
```
python auto_test.py → AI sees → AI fixes → Repeat (2x)
```

**10x faster iteration speed!**
