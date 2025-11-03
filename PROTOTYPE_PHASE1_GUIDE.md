# Steel Cathedral - Phase 1 Prototype Implementation Guide
## Mech Interior with Basic Movement

This guide will walk you through converting the FPS template into a basic mech interior prototype.

---

## Overview
We'll create:
1. A new test level with simple terrain
2. Interior cockpit geometry with large windows
3. A pilot station the player can interact with
4. A basic mech exterior (visible through windows)
5. Simple mech movement controls

---

## Part 1: Create New Level and Clean Slate

### Step 1.1: Create New Level
1. **File → New Level**
2. Select **Empty Level**
3. Save as: `Content/Mech/Maps/LVL_MechPrototype.umap`

### Step 1.2: Add Basic Lighting
1. In the level, add:
   - **Directional Light** (sunlight)
     - Intensity: 10
     - Light Color: Slight yellow tint (255, 248, 220)
     - Rotation: (-50, 0, 0) for overhead sun
   - **Sky Light**
     - Intensity: 1.0
     - Light Color: Light blue (200, 220, 255)
   - **Sky Atmosphere** (for realistic sky)
   - **Volumetric Cloud** (optional, for atmosphere)

### Step 1.3: Add Post Process Volume
1. Add **Post Process Volume**
2. Set **Infinite Extent (Unbound)** to **True**
3. Enable:
   - Auto Exposure
   - Bloom (Intensity: 0.5)
   - Ambient Occlusion

---

## Part 2: Create Simple Exterior Terrain

### Step 2.1: Ground Plane
1. Add **Plane** static mesh actor
2. Transform:
   - Location: (0, 0, 0)
   - Scale: (100, 100, 1) - Creates 10,000 x 10,000 cm ground
3. Create material **M_Ground_Simple**:
   - Base Color: Gray-brown (128, 120, 100)
   - Roughness: 0.8
   - Apply to plane

### Step 2.2: Add Urban Ruins (Simple Geometry)
Using basic cube static meshes, create building ruins:

**Building Ruin 1:**
- Cube mesh
- Location: (3000, 2000, 250)
- Scale: (5, 8, 5) - Collapsed building
- Material: Concrete (dark gray, roughness 0.9)

**Building Ruin 2:**
- Cube mesh
- Location: (-2000, 3000, 400)
- Scale: (6, 6, 8)
- Rotation: (0, 0, 15) - Leaning building
- Material: Concrete

**Rubble Piles (3-4 small cubes):**
- Scattered around at various locations
- Scale: (1, 1, 0.5) to (2, 2, 1)
- Slight rotations for organic look

---

## Part 3: Build Mech Exterior (Placeholder)

### Step 3.1: Create BP_PlayerMechPawn
1. **Content Browser → Mech/Blueprints folder**
2. **Right-click → Blueprint Class → Pawn**
3. Name: `BP_PlayerMechPawn`

### Step 3.2: Add Mech Body Components
Open BP_PlayerMechPawn, add these components:

**Root:**
- **Scene Component** named "MechRoot"
- Location: (0, 0, 0)

**Body Parts (Static Mesh Components):**

1. **Torso:**
   - Parent: MechRoot
   - Static Mesh: Cube
   - Location: (0, 0, 2000) - 20 meters up
   - Scale: (8, 6, 10) - 800cm wide, 600cm deep, 1000cm tall
   - Material: Dark metal (50, 50, 55), Metallic: 0.8, Roughness: 0.6

2. **Head/Cockpit:**
   - Parent: Torso
   - Static Mesh: Cube
   - Location: (200, 0, 600) - Front top of torso
   - Scale: (3, 4, 2)
   - Material: Same as torso

3. **Leg_Left:**
   - Parent: MechRoot
   - Static Mesh: Cube
   - Location: (-200, -150, 800) - Bottom left
   - Scale: (2, 2, 8) - Long vertical
   - Material: Metal

4. **Leg_Right:**
   - Parent: MechRoot
   - Static Mesh: Cube
   - Location: (-200, 150, 800)
   - Scale: (2, 2, 8)
   - Material: Metal

5. **Arm_Left:**
   - Parent: Torso
   - Static Mesh: Cube
   - Location: (0, -400, 200) - Side of torso
   - Scale: (1.5, 1.5, 6)
   - Material: Metal

6. **Arm_Right:**
   - Parent: Torso
   - Static Mesh: Cube
   - Location: (0, 400, 200)
   - Scale: (1.5, 1.5, 6)
   - Material: Metal

7. **CapsuleComponent** (for collision):
   - Parent: MechRoot
   - Location: (0, 0, 2000)
   - Capsule Radius: 400
   - Capsule Half Height: 2100

---

## Part 4: Build Interior Cockpit

### Step 4.1: Create Interior Environment Actor
1. **Create new Blueprint: BP_InteriorEnvironment**
2. Parent class: **Actor**

### Step 4.2: Add Interior Geometry
Inside BP_InteriorEnvironment, add these Static Mesh Components:

**Floor:**
- Mesh: Cube
- Location: (0, 0, -50)
- Scale: (6, 5, 0.1) - Thin floor panel
- Material: **M_Interior_Floor**
  - Base Color: Dark gray (40, 40, 45)
  - Metallic: 0.3
  - Roughness: 0.7

**Walls (Left/Right/Back):**

*Left Wall:*
- Mesh: Cube
- Location: (0, -250, 100)
- Scale: (6, 0.1, 2)
- Material: **M_Interior_Wall** (slightly lighter gray)

*Right Wall:*
- Mesh: Cube
- Location: (0, 250, 100)
- Scale: (6, 0.1, 2)

*Back Wall:*
- Mesh: Cube
- Location: (-300, 0, 100)
- Scale: (0.1, 5, 2)

**Ceiling:**
- Mesh: Cube
- Location: (0, 0, 250)
- Scale: (6, 5, 0.1)
- Material: Interior Wall

### Step 4.3: Add Cockpit Window (CRITICAL)
This is what makes the mech feel real - seeing outside!

**Window Frame:**
- Mesh: Cube
- Location: (310, 0, 100) - Front wall position
- Scale: (0.2, 4.5, 1.8) - Frame around window
- Material: Dark metal

**Window Glass:**
1. Create material: **M_Cockpit_Glass**
   - Material Domain: Surface
   - Blend Mode: Translucent
   - Base Color: (200, 220, 255) - Slight blue tint
   - Metallic: 0.1
   - Roughness: 0.05
   - Opacity: 0.15 (mostly transparent)
   - Refraction: 1.02 (slight distortion)

2. Add Static Mesh Component: **WindowGlass**
   - Mesh: Plane
   - Location: (300, 0, 100)
   - Rotation: (0, 90, 0) - Facing forward
   - Scale: (1.8, 4.2, 1) - Large window!
   - Material: M_Cockpit_Glass

**The window should be HUGE - 15 meters wide, 8 meters tall in final version. For prototype, make it fill most of the front view.**

### Step 4.4: Add Interior Lighting
Inside BP_InteriorEnvironment:

**Emergency Lights (2-3 Point Lights):**
- Location: Scattered on ceiling
- Light Color: Red (255, 50, 50)
- Intensity: 500
- Attenuation Radius: 400

**Cockpit Console Light:**
- Location: (100, 0, -30) - On floor/console area
- Light Color: Blue (100, 150, 255)
- Intensity: 300
- Attenuation Radius: 300

---

## Part 5: Create Pilot Station

### Step 5.1: Create BP_PilotStation
1. **Create Blueprint: BP_PilotStation**
2. Parent: **Actor**
3. Implements: **BPI_Interactable** (create this interface)

### Step 5.2: Add Station Components

**Control Console:**
- Static Mesh: Cube
- Location: (0, 0, 0)
- Scale: (1.5, 2, 0.8) - Console desk
- Material: Dark metal with blue glow accents

**Pilot Seat:**
- Static Mesh: Cube
- Location: (-50, 0, 50)
- Scale: (0.8, 0.8, 0.5)
- Material: Dark leather/fabric

**Camera Component:**
- Name: PilotCamera
- Location: (0, 0, 120) - Eye level when seated
- Field of View: 90

**Interaction Trigger:**
- Box Component
- Location: (-100, 0, 0)
- Extents: (100, 100, 100)

### Step 5.3: Add Pilot Station Variables
In BP_PilotStation, create these variables:

- `IsOccupied` (Boolean) - Default: False
- `OccupyingPlayer` (Actor Reference) - Default: None
- `OwningMech` (BP_PlayerMechPawn Reference)

### Step 5.4: Pilot Station Logic

**Event Graph:**

*Function: Interact (from interface)*
```
When player presses [E] near station:
→ Check: Is NOT Occupied
  → TRUE:
    → Set IsOccupied = True
    → Set OccupyingPlayer = Interacting Player
    → Disable player character movement
    → Attach player camera to PilotCamera
    → Enable mech control input
    → Show on-screen message: "Press [E] to Exit"
  → FALSE:
    → Show message: "Station occupied"
```

*Function: ExitStation*
```
When player presses [E] while seated:
→ Set IsOccupied = False
→ Re-enable player character movement
→ Detach camera, return to player
→ Disable mech control input
→ Set OccupyingPlayer = None
```

---

## Part 6: Add Mech Movement Component

### Step 6.1: Create Custom Component
1. In BP_PlayerMechPawn, **Add Component → Actor Component**
2. Name: **MechMovementComponent**

### Step 6.2: Movement Component Variables
Create these variables in the component:

**Movement Parameters:**
- `MaxWalkSpeed` (Float) - Default: 800.0
- `TurnRate` (Float) - Default: 30.0
- `CurrentSpeed` (Float) - Default: 0.0
- `MovementState` (Enum: Idle, Walking, Running) - Default: Idle

**Input Values:**
- `MoveForwardInput` (Float) - Default: 0.0
- `TurnInput` (Float) - Default: 0.0

### Step 6.3: Movement Logic

**Tick Function:**
```blueprint
Every Tick:
→ If MoveForwardInput != 0:
  → CurrentSpeed = Lerp(CurrentSpeed, MaxWalkSpeed, DeltaTime * 2.0)
  → MovementState = Walking
→ Else:
  → CurrentSpeed = Lerp(CurrentSpeed, 0, DeltaTime * 3.0)
  → MovementState = Idle

→ Calculate Forward Movement:
  → MoveVector = ForwardVector * CurrentSpeed * DeltaTime
  → AddActorWorldOffset(MoveVector, True)

→ If TurnInput != 0:
  → RotationDelta = TurnInput * TurnRate * DeltaTime
  → AddActorWorldRotation(0, RotationDelta, 0)
```

---

## Part 7: Set Up Input System

### Step 7.1: Create Mech Input Actions
1. **Content Browser → Input/Actions folder**
2. Create new Input Actions:
   - **IA_Mech_Move** (Axis2D)
   - **IA_Mech_Turn** (Axis1D)
   - **IA_Mech_Interact** (Button)

### Step 7.2: Create Mech Input Mapping Context
1. Create: **IMC_MechControls**
2. Add mappings:
   - **IA_Mech_Move** → W/A/S/D keys
     - W: (1, 0)
     - S: (-1, 0)
     - A: (0, -1)
     - D: (0, 1)
   - **IA_Mech_Turn** → Mouse X-axis
   - **IA_Mech_Interact** → E key

### Step 7.3: Input Processing in BP_PlayerMechPawn

**Event Graph:**

*IA_Mech_Move (Enhanced Input Action):*
```
→ Get Action Value (Vector2D)
→ Forward = Value.X
→ Right = Value.Y
→ Call MechMovementComponent → Set Move Forward Input(Forward)
```

*IA_Mech_Turn (Enhanced Input Action):*
```
→ Get Action Value (Float)
→ Call MechMovementComponent → Set Turn Input(Value)
```

---

## Part 8: Nest Interior Inside Mech

### Step 8.1: Add Interior to Mech Pawn
1. Open **BP_PlayerMechPawn**
2. **Add Component → Child Actor Component**
3. Name: **InteriorEnvironment**
4. Child Actor Class: **BP_InteriorEnvironment**
5. Location: (200, 0, 2500) - Inside the head/cockpit
6. This keeps interior and exterior in sync!

### Step 8.2: Add Pilot Station to Interior
1. In **BP_InteriorEnvironment**
2. **Add Component → Child Actor Component**
3. Child Actor Class: **BP_PilotStation**
4. Location: (0, 0, 0) - Center of cockpit
5. Set OwningMech reference to parent BP_PlayerMechPawn

---

## Part 9: Camera and View Setup

### Step 9.1: Configure Pilot Camera
In BP_PilotStation → PilotCamera component:
- **Field of View:** 90 (wide for immersion)
- **Aspect Ratio:** 16:9
- **Near Clip Plane:** 10 (prevent clipping interior)

### Step 9.2: Window View Setup
The window glass should render the exterior world normally. Ensure:
1. Window glass material has Lighting Mode: Surface Translucency Volume
2. Opacity is low (0.1 - 0.2) so exterior is clearly visible
3. Position camera so window fills most of the view when seated

---

## Part 10: Place Mech in Level and Test

### Step 10.1: Place Mech
1. Drag **BP_PlayerMechPawn** into LVL_MechPrototype
2. Location: (0, 0, 0)
3. Ensure legs are on ground plane

### Step 10.2: Set Player Start
1. Add **Player Start** actor
2. Location: Inside the mech cockpit, near pilot station
   - Approximately: (200, 0, 2620) - standing in front of pilot seat
3. Rotation: Facing the pilot station

### Step 10.3: Configure Game Mode
1. Open **World Settings**
2. Set Game Mode Override: **BP_FirstPersonGameMode** (or create new BP_MechGameMode)
3. Default Pawn Class: Keep as **BP_FirstPersonCharacter** (player walks to station)

---

## Part 11: Testing Sequence

### Test 1: Basic Level
1. **Play in Editor (Alt+P)**
2. Verify:
   - Player spawns inside mech
   - Can see outside through window
   - Terrain and ruins visible
   - Lighting looks good

### Test 2: Pilot Station Interaction
1. Walk to pilot station
2. Press **E** to interact
3. Verify:
   - Camera switches to PilotCamera
   - Can see pilot view through windscreen
   - Player movement disabled
   - Ready to control mech

### Test 3: Mech Movement
1. While seated at pilot station:
2. Press **W** - Mech moves forward
3. Press **A/D** or **Mouse** - Mech turns
4. Verify:
   - Entire mech (interior + exterior) moves together
   - Terrain visible through window moves correctly
   - Movement feels heavy and slow
5. Press **E** - Exit pilot station

---

## Part 12: Polish for Prototype

### Add Simple UI
Create Widget Blueprint: **WBP_MechHUD**
- Show text: "Speed: [CurrentSpeed]"
- Show text: "Press E to [Enter/Exit] Pilot Station"
- Show crosshair at screen center

### Add Audio (Optional)
1. Find free metal sound effects
2. Add to MechMovementComponent:
   - **Footstep sound** - Play when moving
   - **Hydraulic hiss** - Loop when mech active
   - **Servo whine** - On rotation

### Add Camera Shake
1. Create Camera Shake asset: **CS_MechFootstep**
2. In MechMovementComponent, trigger shake on each "footstep"
3. Makes movement feel weighty

---

## Expected Result

When complete, you should have:
- ✅ A 42-meter tall mech visible from outside
- ✅ Interior cockpit you can walk around in
- ✅ Massive window showing the outside world
- ✅ Pilot station you can sit at
- ✅ WASD controls to move the entire mech
- ✅ Mouse to turn the mech
- ✅ Exterior terrain visible as mech moves
- ✅ Everything synchronized (interior stays inside exterior)

---

## Troubleshooting

**Problem: Can't see outside through window**
- Check window glass material opacity (should be 0.1-0.2)
- Check window is positioned in front of cockpit
- Ensure window rotation is correct (0, 90, 0)

**Problem: Interior doesn't move with exterior**
- Verify InteriorEnvironment is Child Actor of PlayerMechPawn
- Check attachment is set correctly

**Problem: Mech movement too fast**
- Reduce MaxWalkSpeed in MechMovementComponent
- Increase interpolation time in movement lerp

**Problem: Controls don't work**
- Verify Input Mapping Context is added when entering pilot station
- Check Input Actions are bound in Event Graph
- Ensure Pilot Station sets input mode correctly

---

## Next Steps (After Basic Prototype Works)

1. **Add Gunner station** in weapons bay
2. **Add Technician character** that can walk around
3. **Add reactor system** with heat/power mechanics
4. **Add more detailed mech exterior** (arms, weapons)
5. **Add enemy mech** to test combat
6. **Add hazard system** (fires, leaks)

---

**This prototype validates the core concept: Operating a massive mech from inside while seeing the outside world through huge windows.**
