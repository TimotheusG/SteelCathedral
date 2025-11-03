# Automated Screenshot Testing

This system allows you to automatically capture screenshots from multiple angles for AI analysis.

## Quick Start

### Method 1: In-Editor (Recommended)

1. **Open Unreal Editor**
2. **Press Play** (Alt+P or click Play button)
3. **Open Console** (press ` or ~)
4. **Type these commands:**
   ```
   SpawnActor ScreenshotCapture
   ke * StartCaptureSequence
   ```

This will automatically capture 7 screenshots from different angles:
- Front view of mech
- Left and right side views
- Top-down view
- Diagonal overview
- Interior cockpit view
- Close-up of head/cockpit

### Method 2: Manual Screenshots

1. Open Unreal Editor
2. Press Play
3. Use WASD to move around
4. Press **F9** to take a screenshot
5. Screenshots save to: `Saved/Screenshots/WindowsEditor/`

## Using Screenshots for Analysis

After capturing screenshots, run:

```bash
python Tools/collect_screenshots.py
```

This will copy recent screenshots to the project root with timestamps, making them easy to share with the AI assistant for analysis.

## What Gets Captured

The automated system captures:

1. **front_view.png** - Shows the mech from the front
2. **left_side.png** - Left side profile
3. **right_side.png** - Right side profile
4. **top_down.png** - Bird's eye view
5. **diagonal_overview.png** - Isometric-style overview
6. **interior_cockpit.png** - View from inside the cockpit
7. **closeup_head.png** - Close-up of the mech's head/cockpit

## Screenshot Locations

- **During Play:** `Saved/Screenshots/WindowsEditor/`
- **After Collection:** Project root with timestamp

## Integration with Testing

The test runner (`run_tests.py`) now includes screenshot support:

```bash
python Tools/run_tests.py
```

Options:
- **[V]** Run quick validation
- **[S]** Show screenshot instructions
- **[T]** Run full Unreal tests
- **[Q]** Quit

## Troubleshooting

### Screenshots are all black
- Make sure you're in Play mode (PIE)
- Ensure lighting is set up in the level
- Check that the mech is actually spawning

### Can't spawn ScreenshotCapture
- Make sure you compiled the project
- Check that SteelCathedralsEditor.dll was built successfully
- Try closing and reopening the editor

### Screenshots not appearing
- Check `Saved/Screenshots/WindowsEditor/` directory
- Look for error messages in the Output Log window
- Make sure you have write permissions to the project directory

## Advanced Usage

### Custom Angles

In the console, you can capture from specific positions:

```
ke ScreenshotCapture CaptureFromAngle X=1000 Y=0 Z=2000 Pitch=0 Yaw=180 Roll=0 "my_custom_angle"
```

### Adjusting Capture Distance

Edit `ScreenshotCapture` actor properties:
- `CaptureDistance` - How far from target (default: 3000cm = 30m)
- `CaptureHeight` - Height for side views (default: 2000cm = 20m)

## Why This Matters

This automated screenshot system allows the AI assistant to:
- **See actual geometry** instead of guessing from code
- **Identify clipping issues** between interior/exterior
- **Validate spawn positions** visually
- **Debug visual problems** faster
- **Iterate without constant back-and-forth**

Instead of:
1. User: "It looks wrong"
2. AI: "What's wrong?"
3. User: "The thing is clipping"
4. AI: "Which thing?"
5. *repeat 10 times*

We get:
1. User: *runs screenshot capture*
2. AI: *sees screenshots* "Ah, the interior is 500cm too high, fixing..."
3. Done!

## Next Steps

After capturing screenshots, simply mention them to the AI:
- "Check the screenshots I just took"
- "Look at screenshot_front_view_20251103_175500.png"
- "The new screenshots are ready"

The AI can read image files and provide visual analysis!
