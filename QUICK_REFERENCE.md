# Steel Cathedral - Quick Reference
## Prototype Phase 1 - Key Values and Specifications

---

## Mech Scale

| Component | Height (cm) | Height (m) | Width (cm) | Depth (cm) |
|-----------|-------------|------------|------------|------------|
| **Full Mech** | 4200 | 42 | 800 | 600 |
| Torso | 1000 | 10 | 800 | 600 |
| Head/Cockpit | 200 | 2 | 300 | 400 |
| Each Leg | 1600 | 16 | 200 | 200 |
| Each Arm | 600 | 6 | 150 | 150 |

**Note:** Unreal Engine units are in centimeters by default.

---

## Interior Cockpit Dimensions

| Component | Size (cm) | Notes |
|-----------|-----------|-------|
| Floor | 600 x 500 | Walking space |
| Ceiling Height | 250 | Player is ~180cm tall |
| Window Width | 1500 | 15 meters wide! |
| Window Height | 800 | 8 meters tall! |
| Cockpit Location | (200, 0, 2500) | Inside mech head |

---

## Movement Parameters

| Parameter | Value | Unit | Notes |
|-----------|-------|------|-------|
| **MaxWalkSpeed** | 800 | cm/s | ~18 mph, feels slow for 42m mech |
| **TurnRate** | 30 | degrees/s | Deliberate turning |
| **Acceleration** | 2.0 | Lerp factor | Ramp up speed |
| **Deceleration** | 3.0 | Lerp factor | Faster stop |

---

## Camera Settings

| Setting | Value | Notes |
|---------|-------|-------|
| Field of View | 90° | Wide for immersion |
| Near Clip Plane | 10 cm | Prevent interior clipping |
| Camera Height (Seated) | 120 cm | Eye level at station |

---

## Colors (RGB)

### Materials
- **Mech Exterior Metal:** (50, 50, 55) - Dark gunmetal
- **Interior Floor:** (40, 40, 45) - Darker gray
- **Interior Walls:** (60, 60, 65) - Slightly lighter
- **Emergency Lights:** (255, 50, 50) - Red
- **Console Glow:** (100, 150, 255) - Blue
- **Window Glass Tint:** (200, 220, 255) - Light blue

### Material Properties
- **Metal:** Metallic: 0.8, Roughness: 0.6
- **Interior Panels:** Metallic: 0.3, Roughness: 0.7
- **Window Glass:** Opacity: 0.15, Refraction: 1.02

---

## Lighting

### Directional Light (Sun)
- **Intensity:** 10
- **Color:** (255, 248, 220) - Warm daylight
- **Rotation:** (-50, 0, 0)

### Sky Light
- **Intensity:** 1.0
- **Color:** (200, 220, 255) - Blue sky

### Emergency Lights (Interior)
- **Intensity:** 500
- **Attenuation Radius:** 400 cm
- **Color:** Red

### Console Light (Interior)
- **Intensity:** 300
- **Attenuation Radius:** 300 cm
- **Color:** Blue

---

## Terrain Layout

| Feature | Location (X, Y, Z) | Scale |
|---------|-------------------|-------|
| Ground Plane | (0, 0, 0) | (100, 100, 1) |
| Building Ruin 1 | (3000, 2000, 250) | (5, 8, 5) |
| Building Ruin 2 | (-2000, 3000, 400) | (6, 6, 8) |
| Rubble Piles | Scattered | (1-2, 1-2, 0.5-1) |

**Ground Size:** 10,000 x 10,000 cm (100m x 100m)

---

## Input Mapping

### Mech Controls (When Piloting)
| Input | Action | Value Type |
|-------|--------|------------|
| **W** | Move Forward | +1.0 |
| **S** | Move Backward | -1.0 |
| **A** | Strafe Left | (0, -1) |
| **D** | Strafe Right | (0, +1) |
| **Mouse X** | Turn | Axis1D |
| **E** | Interact/Exit Station | Button |

### Player Controls (When Walking)
| Input | Action |
|-------|--------|
| **WASD** | Walk |
| **Mouse** | Look |
| **Space** | Jump |
| **E** | Interact with Station |

---

## Component Hierarchy

```
BP_PlayerMechPawn (Pawn)
├── MechRoot (SceneComponent)
│   ├── Torso (StaticMesh)
│   │   ├── Head (StaticMesh)
│   │   ├── Arm_Left (StaticMesh)
│   │   └── Arm_Right (StaticMesh)
│   ├── Leg_Left (StaticMesh)
│   ├── Leg_Right (StaticMesh)
│   ├── CapsuleComponent (Collision)
│   └── InteriorEnvironment (ChildActor)
│       ├── Floor (StaticMesh)
│       ├── Walls (StaticMesh x3)
│       ├── Ceiling (StaticMesh)
│       ├── WindowGlass (StaticMesh)
│       ├── EmergencyLights (PointLight x3)
│       └── PilotStation (ChildActor)
│           ├── Console (StaticMesh)
│           ├── Seat (StaticMesh)
│           ├── PilotCamera (Camera)
│           └── InteractionTrigger (BoxComponent)
```

---

## Blueprint Variables Quick Reference

### BP_PlayerMechPawn
- None (uses component)

### MechMovementComponent
- `MaxWalkSpeed` (Float): 800.0
- `TurnRate` (Float): 30.0
- `CurrentSpeed` (Float): 0.0
- `MovementState` (Enum): Idle
- `MoveForwardInput` (Float): 0.0
- `TurnInput` (Float): 0.0

### BP_PilotStation
- `IsOccupied` (Bool): False
- `OccupyingPlayer` (Actor Reference): None
- `OwningMech` (BP_PlayerMechPawn Reference): Set at runtime

---

## File Organization

```
Content/
├── Mech/
│   ├── Blueprints/
│   │   ├── BP_PlayerMechPawn.uasset
│   │   ├── BP_InteriorEnvironment.uasset
│   │   └── BP_PilotStation.uasset
│   ├── Materials/
│   │   ├── M_MechMetal.uasset
│   │   ├── M_Interior_Floor.uasset
│   │   ├── M_Interior_Wall.uasset
│   │   └── M_Cockpit_Glass.uasset
│   ├── Maps/
│   │   └── LVL_MechPrototype.umap
│   └── UI/
│       └── WBP_MechHUD.uasset
├── Input/
│   ├── Actions/
│   │   ├── IA_Mech_Move.uasset
│   │   ├── IA_Mech_Turn.uasset
│   │   └── IA_Mech_Interact.uasset
│   └── IMC_MechControls.uasset
```

---

## Testing Checklist

- [ ] Player spawns inside mech cockpit
- [ ] Can see outside world through window
- [ ] Terrain and ruins visible at correct scale
- [ ] Can walk to pilot station
- [ ] Press E to enter pilot station
- [ ] Camera switches to pilot view
- [ ] Press W to move mech forward
- [ ] Press A/D or Mouse to turn mech
- [ ] Exterior world moves correctly through window
- [ ] Press E to exit pilot station
- [ ] Return to walking mode

---

## Common Issues & Solutions

| Problem | Solution |
|---------|----------|
| Window is black/can't see through | Check glass material opacity (0.15), ensure Translucent blend mode |
| Interior doesn't move with mech | Interior must be ChildActor component of MechPawn |
| Controls don't work | Add IMC_MechControls to Input Mapping Context stack when entering station |
| Mech moves too fast | Reduce MaxWalkSpeed from 800 to 400-600 |
| Scale feels wrong | Verify mech height is 4200cm (42m), player is ~180cm |
| Camera clips through walls | Increase Near Clip Plane to 10-15cm |

---

## Performance Targets (Prototype)

| Metric | Target |
|--------|--------|
| Frame Rate | 60+ FPS |
| Draw Calls | < 500 |
| Triangles | < 100K |
| Memory | < 2GB |

---

## Next Phase Features

After Phase 1 prototype works:
1. **Gunner Station** - Separate weapons control
2. **Technician Character** - Walking repairs
3. **Reactor System** - Heat/power management
4. **Enemy Mech** - Basic combat
5. **Weapon Firing** - Visual/audio feedback
6. **Hazard Spawning** - Interior damage
7. **Window Cracking** - Visual feedback

---

**Keep this document open while building the prototype for quick reference!**
