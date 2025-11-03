# Steel Cathedral — Version 1.0
A three-player co-op game about operating a combat mech from the inside.

## Core Concept
Three players man different stations inside a massive walking mech during combat. The exterior may be armored, but the interior is a pressure cooker of heat, smoke, and system failures. Success requires coordination and timing.

## Roles

### 1. Pilot — Movement Control
- Controls walking, turning, balance, and stance
- Manages momentum—sharp stops or turns destabilize the platform
- Limited forward visibility; depends on callouts from the Gunner
- Can activate boost thrusters to dodge, but they spike heat
- Handles terrain positioning: cover, elevation, line of sight
- Feels every impact through the controls

**Player experience:** Operating something massive and powerful that responds slowly and can tip at any moment.

### 2. Gunner — Weapons Systems
- Operates cannons, missiles, and energy weapons via targeting cameras
- Must time shots with stable footing—firing mid-stride throws off accuracy
- Weapon types:
  - Kinetic rounds: Low heat, weak penetration
  - Plasma beams: High heat, precise, slow charge
  - Missiles: Moderate heat, area damage, limited ammo
- Every shot increases heat and drains power
- Coordinates constantly with the Technician on thermal limits

**Player experience:** Precision under pressure, with immediate feedback when shots connect.

### 3. Technician — Systems Maintenance
- Monitors reactor heat, pressure, and power distribution
- Performs physical repairs:
  - Fixes circuits, extinguishes fires, seals coolant leaks
  - Manually vents heat through reactor hatches
  - Reroutes power from non-critical systems
- Manual venting releases steam that obscures visibility and drops power output
- Constantly moving between compartments with tools

**Player experience:** Fighting entropy in real time while the machine tries to tear itself apart.

## Game Flow

### Launch Sequence
Boot systems, run diagnostics, power the reactor. The mech stands and stabilizes. The enemy appears.

### Combat Phase
The Pilot positions. The Gunner fires when stable. Heat and power drain with each action. The Technician keeps systems functional. Enemy hits cause internal damage: electrical fires, ruptured lines, console failures.

### Crisis Phase
Reactor approaches critical. Lights flicker. A system fails—weapons, stabilizers, or sensors. The Technician prioritizes what to save while the Pilot maintains balance and the Gunner lines up a final shot.

### Resolution
Either the enemy goes down or the reactor fails. The outcome plays through the interior: rising heat, shaking metal, or sudden silence.

## Core Systems

| System | Owner | Function |
|--------|-------|----------|
| Movement | Pilot | Position and balance; affects aim and reactor load |
| Weapons | Gunner | Primary heat/power drain; requires Tech coordination |
| Reactor | Technician | Generates all power; heat rises with every major action |
| Cooling | Technician | Manual venting; risky but necessary |
| Damage Control | Technician | Repairs hazards from enemy fire |
| Communication | All | The difference between function and failure |

## Win/Lose Conditions

**Victory:** Destroy the enemy before critical failure.

**Defeat:**
- Reactor meltdown
- Total power loss
- Catastrophic internal damage

## Atmosphere
Industrial interior. Tight spaces. Moving machinery. Red emergency lighting, constant vibration, metal stress sounds. Communication is survival. Audio cues matter: reactor hum means stability, venting hiss means relief, system silence means trouble.

## Example Encounter
A bipedal enemy mech fires plasma that melts hull plating. The Pilot dodges while maintaining line of sight. The Gunner's railgun can penetrate armor but overheats the reactor with each shot. The Technician battles to keep heat below 95% while fighting a corridor fire. One more vent cycle, one more stable shot. Either the enemy falls or the core detonates.

## Design Goal
The tension comes from operating a machine that's barely functional. Systems don't accommodate you. Only coordination keeps it moving.

---

# Steel Cathedral — Technical Design Document

## Table of Contents
1. Executive Summary
2. Environmental Design
3. Interior Architecture
4. Exterior Design & Scale
5. Combat & Movement Systems
6. Enemy Behavior
7. Technical Architecture (UE5)
8. System Organization
9. Implementation Phases

---

## 1. Executive Summary
Steel Cathedral is a three-player co-op experience built in Unreal Engine 5 where players operate stations inside a 40-meter tall combat mech. The game emphasizes physicality, spatial awareness, and coordination under pressure. Players experience combat from inside the machine—not through HUD elements, but through environmental feedback: shaking floors, sparking panels, visible external threats through reinforced windows, and the constant presence of the battlefield outside.

---

## 2. Environmental Design

### Terrain Types & Combat Influence

#### Urban Ruins
- Collapsed buildings provide cover but restrict movement lanes
- Pilot must navigate tight corners—overshooting means exposing the mech's back or getting stuck
- Rubble creates unstable footing; the Technician sees hydraulic pressure spikes on readouts
- Enemy can use buildings for ambush positioning or climb structures for elevation advantage

#### Industrial Wasteland
- Open sightlines favor ranged combat
- Scattered machinery and containers offer partial cover
- Uneven ground causes constant balance adjustments; Pilot feels this as delayed response to inputs
- Pools of toxic runoff—stepping in them causes hull corrosion (visible interior warnings, Hull Integrity gauges drop)

#### Volcanic Caldera
- Extreme ambient heat—reactor operates closer to critical baseline
- Technician must vent more frequently, reducing available power windows for the Gunner
- Lava flows create natural barriers; Pilot must route around or risk leg actuator damage
- Geysers erupt unpredictably, forcing sudden evasive maneuvers that throw off Gunner's aim

#### Frozen Tundra
- Ice reduces traction—Pilot's turning radius increases, bracing becomes mandatory before firing
- Cold increases reactor efficiency but makes manual venting dangerous (ice buildup on valves)
- Snowstorms reduce external visibility through windows; Gunner relies entirely on thermal cameras
- Enemy may break through ice to create pitfalls

#### Canyon Network
- Narrow passages force single-file movement
- High walls limit dodge options—Pilot must commit to forward or reverse
- Echoing sounds make it hard to track enemy position by audio alone
- Ricochets from canyon walls can hit the mech from unexpected angles, causing internal damage in "safe" compartments

### Terrain Interaction Systems

#### Dynamic Deformation
- Mech footsteps leave craters, crush vehicles, collapse structures
- Terrain can fail under mech weight (thin ice, weakened floors, bridge spans)

#### Environmental Hazards
- Electrical storms (power surges, require Technician to isolate systems)
- Sandstorms (reduced visibility, sand intrusion into vents causes overheating)
- Acid rain (gradual hull degradation, Technician sees integrity warnings)

#### Elevation & Line of Sight
- High ground grants Gunner better targeting angles but exposes more of the mech to return fire
- Pilot can crouch to reduce silhouette but sacrifices mobility
- Enemy may climb or leap to rooftops, forcing Gunner to track vertical movement

---

## 3. Interior Architecture

### Scale & Layout
The mech interior is three stories tall and spans roughly 60 meters from cockpit to reactor bay. Players traverse catwalks, ladders, and narrow corridors. The space feels industrial and utilitarian—no comfort, only function.

### Key Compartments

#### Cockpit (Front, Top Level)
- **Pilot's station:** wraparound control yoke, pedal controls, throttle levers
- **Windscreen:** 15 meters wide, 8 meters tall, reinforced triple-layered glass
  - Provides panoramic view of battlefield
  - Shows enemies at full scale—a 30-meter tall kaiju fills the entire window when close
  - Cracks spread across glass when hit directly; structural integrity warnings flash
- **Instrument cluster:** mechanical gauges for speed, balance, hydraulic pressure
- Limited peripheral vision—relies on Gunner for bearing callouts

#### Weapons Bay (Mid-Level, Starboard)
- **Gunner's station:** targeting console, physical trigger controls, ammunition feeds visible overhead
- **Observation ports:** 3-meter diameter circular windows on each side
  - Shows weapon trajectories and impact points
  - Missiles streak past these windows during volleys
  - When enemy closes to melee range, claws and limbs visible scraping against glass
- **Screens display** camera feeds from external turrets
- Ammunition magazines cycle visibly—mechanical arms load shells into breach systems

#### Reactor Core (Aft, Bottom Level)
- **Technician's primary workspace:** central reactor column, 10 meters tall, glowing blue-white
- Heat radiates visibly—air shimmers, steam vents periodically
- Control panels on all sides: breakers, valves, pressure readouts, coolant gauges
- **Rear observation window:** 5 meters wide, shows what's behind the mech
  - Critical for detecting flanking enemies
  - Technician sees impacts on mech's back and can warn Pilot
- Coolant pipes run floor to ceiling; leaks spray visible vapor
- Emergency venting stations require manual crank wheels

#### Engineering Corridors (Connecting All Compartments)
- Narrow—1.5 meters wide—forcing single-file movement
- Exposed systems: electrical conduits, hydraulic lines, steam pipes
- Ladders connect vertical levels
- **Side view ports:** Small reinforced windows every 10 meters
  - Allow glimpses of battlefield while moving between stations
  - Technician running to fix a fire sees a missile impact the mech's shoulder while climbing a ladder
- Lighting: red emergency strips, flickering fluorescents, sparking panels
- Environmental hazards spawn here: electrical fires, coolant floods, collapsed grating

#### Auxiliary Systems (Distributed)
- **Life Support:** Upper corridor access; can be rerouted to boost reactor efficiency
- **Hydraulic Pump Room:** Mid-level; controls leg actuators and balance systems
- **Ammunition Storage:** Lower level, near Weapons Bay; vulnerable to enemy fire
- **Backup Generator:** Rear compartment; Technician can manually engage if main reactor fails

### Window Experience
The windows are massive and omnipresent. Players are never more than 20 meters from a view of the outside battle.

#### During Combat
- A missile streaks past the Gunner's observation port, close enough to see individual exhaust trails
- The Technician, running down a corridor, glances through a side port and sees an enemy limb swinging toward the mech—the impact arrives half a second later, throwing them against the wall
- Pilot watches enemy acid spit arc through the air in slow motion toward the windscreen; upon impact, acid sizzles and spreads across the glass, obscuring vision
- When the mech is struck in the arm, the Gunner sees the explosion through their window, feels the compartment shake, hears metal scream—then a structural warning as the minigun arm seals off and ammunition feeds go dark

#### Scale Communication
- A 35-meter tall kaiju fills the cockpit windscreen when it closes to melee range
- Its claws scrape down the glass, each talon 3 meters long, leaving gouges
- The Pilot sees the creature's eyes—each the size of a car—tracking their movements
- When the enemy charges, the ground visible through lower view ports shakes with each footfall

#### Psychological Effect
- Players never forget they're in combat because they see it constantly
- The windows are terrifyingly large—they feel exposed, even though the glass is reinforced
- Cracks spreading across the cockpit windscreen create genuine tension
- Seeing the battlefield while performing interior tasks grounds players in the stakes

---

## 4. Exterior Design & Scale

### Mech Specifications
- **Model Designation:** SC-1 "Cathedral"
- **Height:** 42 meters (14 stories)
- **Weight:** 850 tons
- **Locomotion:** Bipedal, reverse-joint legs
- **Armament:** Twin shoulder-mounted cannons, dorsal missile pods, arm-mounted miniguns, chest-mounted plasma array

### Visual Design

#### Silhouette
- Broad shoulders, narrow waist, heavily armored torso
- Legs thick and industrial—pistons and hydraulics visible
- Head unit small relative to body, housing primary sensors and cockpit
- Asymmetrical damage accumulation (scorching, punctures, torn plating)

#### Surface Detail
- Riveted armor plates with visible weld seams
- Exhaust vents along spine release steam during movement
- Cooling fins deploy from shoulders when overheating
- External damage modeled in real-time: gouges, blast craters, melted sections

#### Visible Systems
- Weapon turrets track independently of torso rotation
- Missile pod covers iris open before firing
- Reactor glow visible through ventilation slats on back
- Hydraulic fluid leaks from damaged leg joints

### Damage Visualization

#### Exterior-to-Interior Connection
- Enemy hits the mech's right arm → players inside see sparks shower from ceiling panels in the corresponding corridor
- Missile strikes the back → Technician's rear window cracks, alarms blare, reactor readouts spike
- Acid spray on windscreen → Pilot's vision obscured, must rely on camera feeds
- Leg damage → entire interior tilts, unsecured objects slide, balance gauges redline

---

## 5. Combat & Movement Systems

### Pilot Movement Mechanics

#### Locomotion States
- **Walking:** Steady, predictable platform for Gunner
- **Running:** Faster repositioning, but oscillating aim penalty
- **Bracing:** Locks legs, provides stable firing platform, but immobilizes mech
- **Strafing:** Lateral movement without turning torso, limited duration
- **Boost Dodge:** Short burst in any direction, massive heat spike, cooldown required

#### Balance System
- Center of mass dynamically calculated based on torso angle, leg position, terrain slope
- Overextension causes stumble animations—interior shakes violently, players thrown off balance
- Recovery requires Pilot to counter-steer and reduce speed
- Falling is catastrophic—total systems failure, mission loss

#### Terrain Adaptation
- Pilot sees terrain quality indicator (stable, loose, hazardous)
- Uneven ground slows movement, increases energy consumption
- Ascending slopes requires increased throttle, reactor load rises
- Descending too quickly risks loss of control

### Gunner Weapon Systems

#### Targeting Mechanics
- Crosshair stability tied to Pilot's movement state
- Reticle bloom during walking, tight grouping when braced
- Lead indicators for moving targets
- Rangefinding laser provides distance readout

#### Weapon Types

**Kinetic Cannons (Shoulder-Mounted)**
- High velocity, ballistic trajectory
- 5-second reload cycle
- Low heat generation
- Effective against armor at medium range

**Plasma Array (Chest-Mounted)**
- Requires 8-second charge
- Pinpoint accuracy, hitscan
- Extreme heat generation (15% per shot)
- Penetrates heavy armor

**Missile Pods (Dorsal)**
- Swarm-fire capable (6-12 missiles)
- Moderate heat (8% per volley)
- Lock-on required (3 seconds)
- Limited ammo (Technician must authorize reloads)

**Miniguns (Arm-Mounted)**
- Continuous fire, point defense
- Low damage per hit, high volume
- Minimal heat, uses reserve power
- Vulnerable to being disabled by limb damage

#### Firing Coordination
- Gunner calls "Brace for shot!" → Pilot enters brace stance → Gunner fires → Pilot resumes movement
- Missed timing = wasted shot or friendly fire risk (stray rounds hitting terrain near allied units)
- Heat management = Gunner asks Tech "Can I fire plasma?" before committing to charge cycle

### Technician Maintenance Loop

#### Routine Tasks (Every 30-60 seconds)
- Check reactor temperature gauge
- Monitor power distribution board
- Inspect coolant pressure levels
- Verify hydraulic system integrity

#### Emergency Responses

**Overheating:**
1. Identify heat source (weapons, movement, external fire)
2. Route to reactor bay, initiate manual vent
3. Vent cycle takes 10 seconds, releases interior steam (obscures vision)
4. Monitor cooldown, clear vent before next cycle

**Electrical Fire:**
1. Locate via sparking cables or smoke
2. Grab extinguisher from wall mount
3. Apply foam, verify power reroute at breaker panel
4. Report system offline to affected player

**Coolant Leak:**
1. Follow vapor trail to breach point
2. Apply emergency sealant (physical interaction)
3. Reroute coolant flow via valve controls
4. Pressure restoration takes 15 seconds

**Power Shortage:**
1. Divert power from non-critical systems (life support, lights, auxiliary sensors)
2. Use breaker panel to reallocate load
3. Inform Pilot and Gunner of reduced capability

#### Physical Interaction
- All actions require moving to specific locations
- Levers, wheels, and switches have tactile feedback (controller rumble, mouse resistance)
- VR optional but designed for it—grabbing valves, pulling levers feels direct

---

## 6. Enemy Behavior

### Enemy Types

#### Rival Mech (Humanoid)
- Similar scale (38-45 meters)
- Uses cover and elevation
- Flanking maneuvers—attempts to circle around
- Ranged weapons: railguns, rockets
- Will brace before firing high-damage weapons (telegraphed)
- Can be staggered by concentrated fire

**Tactics:**
- Advances during player reload windows
- Targets specific mech subsystems (legs to immobilize, arms to disarm)
- Retreats behind cover when heavily damaged
- Deploys smoke grenades to obscure targeting

#### Kaiju (Organic)
- Massive scale (30-50 meters)
- Aggressive, charges directly
- Acid spit: arcing projectile, damages windscreen and armor
- Melee swipes: high damage, can sever mech limbs
- Tail whip: area attack, hits multiple mech sections
- Regenerates slowly if not continuously damaged

**Tactics:**
- Closes distance aggressively
- Uses terrain for cover during approach
- Climbs structures to leap onto mech
- Focuses on cockpit when in melee range
- Enrages at low health—faster, more erratic

#### Swarm Units (Small Bipeds, 8-12 meters)
- Fast, numerous (3-6 units)
- Light armor, destroyed by minigun fire
- Coordinate to attack from multiple angles
- Grapple mech legs, slowing movement
- Plant explosives on joints (Technician must detach them)

**Tactics:**
- Split to flank from opposite sides
- One distracts frontally while others attack rear
- Target exposed hydraulics and coolant lines
- Retreat when outnumbered or outgunned

### Enemy Interaction with Terrain

**Urban Ruins:**
- Mechs peek around corners for ambush shots
- Kaiju bursts through buildings to shortcut routes
- Swarm units use rubble for cover, emerge suddenly

**Volcanic Caldera:**
- Kaiju bathes in lava to heal minor wounds
- Enemy mechs use geysers as visual cover
- Swarm units hide in fissures, surprise from below

**Frozen Tundra:**
- Enemy mechs slide on ice during charges
- Kaiju breaks ice beneath player mech to create pitfall traps
- Swarm units burrow under snow, attack legs from concealment

---

## 7. Technical Architecture (UE5)

### Core Systems Overview
This section outlines the actors, components, subsystems, and managers required to build Steel Cathedral in Unreal Engine 5.

### Actor Hierarchy

#### PlayerMechPawn (Primary Actor)
- **Root component:** USceneComponent (mech center of mass)
- **Child components:**
  - SkeletalMeshComponent (mech exterior visual)
  - PhysicsConstraintComponents (leg joints, torso rotation)
  - AudioComponents (engine hum, hydraulic hiss, alarms)
  - CameraComponents (exterior third-person camera for debugging/replay)

**Purpose:** The physical representation of the mech in the game world. Handles collision, physics, and visual rendering.

#### InteriorEnvironmentActor
- Nested inside PlayerMechPawn
- Contains all interior meshes:
  - Cockpit geometry
  - Weapons bay
  - Reactor room
  - Corridors, ladders, catwalks
- **Child components:**
  - StaticMeshComponents (walls, floors, panels)
  - PointLightComponents (emergency lights, reactor glow, sparking wires)
  - ParticleSystemComponents (steam, sparks, coolant spray)
  - AudioComponents (localized alarms, creaking metal, steam hiss)

**Purpose:** The playable interior space. Remains synchronized with exterior mech orientation and movement.

#### PilotStationActor
- **Location:** Cockpit
- **Child components:**
  - InteractionComponent (sit/stand, grab controls)
  - CameraComponent (first-person pilot view)
  - WidgetComponent (diegetic UI: speed gauge, balance indicator, terrain readout)

**Purpose:** Pilot's control interface. Receives input, sends commands to MechMovementComponent.

#### GunnerStationActor
- **Location:** Weapons Bay
- **Child components:**
  - InteractionComponent
  - CameraComponent (first-person gunner view)
  - WidgetComponent (targeting reticle, ammo count, heat warning)
  - SpringArmComponent (external camera view for targeting screens)

**Purpose:** Gunner's control interface. Receives input, sends commands to WeaponSystemsComponent.

#### TechnicianPlayerCharacter
- Standard ACharacter class
- **Child components:**
  - CapsuleComponent (collision)
  - SkeletalMeshComponent (character model, hands for VR)
  - CameraComponent (first-person view)
  - InteractionComponent (use valves, switches, extinguishers)
  - InventoryComponent (tools: wrench, extinguisher, sealant)

**Purpose:** Mobile player character. Navigates interior, performs physical repairs.

#### EnemyMechActor / KaijuActor / SwarmUnitActor
- **Root:** USceneComponent
- **Child components:**
  - SkeletalMeshComponent (animated model)
  - AIControllerComponent
  - HealthComponent
  - WeaponSystemsComponent (for ranged enemies)
  - PerceptionComponent (sight, hearing)
  - AudioComponents (roars, footsteps, weapon fire)

**Purpose:** Enemy combatants. Use behavior trees for tactical decision-making.

### Component Systems

#### MechMovementComponent (UActorComponent)
**Attached to:** PlayerMechPawn

**Responsibilities:**
- Translates Pilot input into mech locomotion
- Manages movement states: walking, running, bracing, strafing, boosting
- Calculates balance and center of mass
- Applies terrain penalties
- Broadcasts movement events to other systems (e.g., "Brace Engaged" → stabilizes Gunner aim)

**Key Variables:**
- CurrentSpeed (float)
- BalanceValue (float, -1.0 to 1.0)
- MovementState (enum: Idle, Walking, Running, Braced, Boosting)
- TerrainType (enum: Stable, Loose, Hazardous, Ice, Lava)

**Events:**
- OnMovementStateChanged (broadcast to UI, audio, Gunner)
- OnBalanceLost (triggers stumble recovery)
- OnFootstep (plays audio, spawns particle effects, triggers camera shake)

#### WeaponSystemsComponent (UActorComponent)
**Attached to:** PlayerMechPawn

**Responsibilities:**
- Manages all mech weapons (cannons, plasma, missiles, miniguns)
- Processes Gunner firing input
- Calculates aim stability based on Pilot movement state
- Tracks ammunition and heat generation per weapon
- Interfaces with ReactorSystemComponent for power draw

**Key Variables:**
- WeaponArray (TArray<FWeaponData>)
  - Each weapon: Type, Ammo, Heat Per Shot, Reload Time, Damage, Range
- CurrentHeatContribution (float, cumulative)
- AimStabilityModifier (float, set by MechMovementComponent)

**Events:**
- OnWeaponFired (spawns projectile, applies heat, triggers audio/VFX)
- OnWeaponOverheated (locks weapon until cooled)
- OnAmmoEmpty (requires Technician reload authorization)

#### ReactorSystemComponent (UActorComponent)
**Attached to:** PlayerMechPawn

**Responsibilities:**
- Core power generation and heat management
- Accumulates heat from weapons, movement, and environmental factors
- Manages power distribution to all subsystems
- Processes manual venting actions from Technician
- Triggers critical failure states

**Key Variables:**
- ReactorHeat (float, 0-100%)
- ReactorPower (float, 0-100%)
- VentingInProgress (bool)
- PowerAllocationMap (TMap<ESystemType, float>)
  - ESystemType: Weapons, Movement, LifeSupport, Sensors, Auxiliary

**Events:**
- OnHeatThresholdReached (50%, 75%, 90%, 100%)
- OnPowerDepleted (total shutdown warning)
- OnVentingStarted / OnVentingCompleted (steam VFX, power reduction)
- OnMeltdownInitiated (critical failure, mission loss)

#### DamageManagementComponent (UActorComponent)
**Attached to:** PlayerMechPawn

**Responsibilities:**
- Tracks hull integrity per mech section (head, torso, arms, legs)
- Receives damage from enemy attacks
- Spawns interior hazards (fires, leaks, electrical failures)
- Manages subsystem failures (weapon offline, leg damaged, sensor loss)

**Key Variables:**
- HullIntegrityMap (TMap<EMechSection, float>)
- ActiveHazards (TArray<AHazardActor*>)
- DisabledSystems (TArray<ESystemType>)

**Events:**
- OnSectionDamaged (spawns interior hazard, triggers alarm)
- OnCriticalDamage (seals corridor, disables system)
- OnHazardSpawned (creates fire, leak, electrical arc actor)
- OnHazardCleared (Technician repaired)

#### InteractionComponent (UActorComponent)
**Attached to:** Interactable actors (valves, switches, levers, panels)

**Responsibilities:**
- Detects player proximity and focus
- Displays interaction prompts
- Processes interaction input (E key, VR hand grab)
- Triggers associated actions (open valve, flip switch, grab extinguisher)

**Key Variables:**
- InteractionType (enum: Valve, Switch, Lever, Tool, Panel)
- InteractionDuration (float, for held actions)
- RequiredTool (optional, for specific tasks)

**Events:**
- OnInteractionStarted
- OnInteractionCompleted
- OnInteractionFailed (wrong tool, already in use)

### Manager Systems (Singletons / Subsystems)

#### CombatStateManager (UGameInstanceSubsystem)
**Responsibilities:**
- Tracks overall combat phase (Launch, Combat, Crisis, Resolution)
- Manages enemy spawning and behavior orchestration
- Monitors win/lose conditions
- Interfaces with UI for mission briefings and results

**Key Variables:**
- CurrentPhase (enum)
- ActiveEnemies (TArray<AActor*>)
- MissionObjective (FMissionData)

**Events:**
- OnPhaseChanged
- OnEnemyDestroyed
- OnMissionComplete / OnMissionFailed

#### AudioManager (UAudioSubsystem)
**Responsibilities:**
- Manages layered audio: ambient (reactor hum), alarms, impacts, voice lines
- Spatializes 3D sound sources relative to player positions
- Triggers dynamic music based on combat intensity
- Processes radio chatter and player voice lines

**Key Functions:**
- PlaySoundAtLocation(SoundCue, Location)
- TriggerAlarm(AlarmType, Severity)
- UpdateMusicIntensity(ThreatLevel)

#### VFXManager (UParticleSubsystem)
**Responsibilities:**
- Spawns and manages particle effects: steam, sparks, explosions, muzzle flashes
- Handles exterior damage effects visible through windows
- Manages interior hazard VFX (fires, electrical arcs, coolant spray)

**Key Functions:**
- SpawnExplosion(Location, Scale)
- SpawnHazardEffect(HazardType, Location, Duration)
- UpdateReactorGlow(HeatLevel)

#### HazardManager (Custom Manager Actor)
**Responsibilities:**
- Spawns interior hazards based on exterior damage events
- Tracks hazard lifecycle (ignition, spread, containment, extinguished)
- Manages hazard impact on systems (fire reduces power, leak increases heat)

**Key Variables:**
- ActiveHazards (TArray<AHazardActor*>)
- HazardSpawnProbability (TMap<EDamageType, float>)

**Events:**
- OnHazardSpawned (creates fire/leak actor)
- OnHazardProgressed (spreads to adjacent areas)
- OnHazardExtinguished (Technician action)

### Actor Types: Hazards

#### FireHazardActor
- Child of AActor
- **Components:**
  - ParticleSystemComponent (flames)
  - AudioComponent (crackling)
  - BoxComponent (damage volume)
- Spreads along cables and surfaces over time
- Damages nearby systems (reduces power allocation)
- Extinguished by Technician with fire extinguisher

#### CoolantLeakActor
- **Components:**
  - ParticleSystemComponent (vapor spray)
  - DecalComponent (puddle on floor)
  - AudioComponent (hissing)
- Increases reactor heat (+2% per second)
- Sealed by Technician with repair tool

#### ElectricalArcActor
- **Components:**
  - NiagaraComponent (arcing electricity)
  - AudioComponent (buzzing, popping)
  - SphereComponent (damage radius)
- Causes power fluctuations
- Risk of shock damage to Technician (stuns player for 2 seconds)
- Fixed by isolating breaker at electrical panel

### Camera Systems

#### PilotCamera (UCameraComponent)
- **Attached to:** PilotStationActor
- **FOV:** 90°
- **Clipping:** Renders through cockpit windscreen only
- **Post-process:** Minimal, slight vignette during high-G maneuvers

#### GunnerCamera (UCameraComponent)
- **Attached to:** GunnerStationActor
- **FOV:** 80°
- **Clipping:** Renders through weapon bay observation ports
- **Post-process:** Slight desaturation during low power, red tint during critical heat

#### TechnicianCamera (UCameraComponent)
- **Attached to:** TechnicianPlayerCharacter head
- **FOV:** 100° (wider for spatial awareness)
- **Post-process:** Dynamic—steam obscures vision during venting, sparks flash during electrical hazards

#### ExternalCameraFeeds (USceneCaptureComponent2D)
- **Attached to:** Exterior mech hull (multiple locations)
- Renders to textures displayed on interior screens
- Used by Gunner for targeting, Tech for rear/flank awareness
- Can be disabled by damage to camera mount

### UI Systems

#### DiegeticUI (In-World)

**Pilot:**
- Physical gauges: speed, balance, hydraulic pressure
- Terrain quality indicator (color-coded light strip)
- Warning lights for critical systems

**Gunner:**
- Targeting reticle (projected on screen)
- Ammo counter (mechanical flip display)
- Heat warning bar (glowing tube fills with red fluid)
- Weapon selection switches

**Technician:**
- Reactor temperature dial (needle gauge)
- Power distribution board (rows of breakers with indicator lights)
- Coolant pressure gauge (analog dial)
- System status panel (grid of lights: green=OK, yellow=warning, red=critical)

#### ScreenSpaceUI (HUD Overlays)
Minimal usage—only for critical info:
- Player role indicator (top-left, small)
- Interaction prompts (context-sensitive, bottom-center)
- Voice chat status (player name + speaking indicator)

### Networking Architecture (Multiplayer)

#### Replication Strategy
- **PlayerMechPawn:** Replicate position, rotation, movement state
- **Components:** Replicate critical variables (heat, power, ammo)
- **Stations:** Replicate occupancy (who's seated where)
- **Hazards:** Replicate spawn/destroy events
- **Damage:** Server-authoritative, replicate to all clients

#### Role Authorities
- **Pilot:** Authority over MechMovementComponent
- **Gunner:** Authority over WeaponSystemsComponent (targeting)
- **Technician:** Authority over InteractionComponent (repairs)
- **Server:** Authority over ReactorSystemComponent, DamageManagementComponent, enemy AI

#### Optimization
- **Tick rate:** 60Hz for player actions, 20Hz for environmental updates
- **Relevancy:** Exterior enemies replicate at lower frequency when not in direct view
- **Hazards:** Spawn client-side VFX, replicate state changes only

---

## 8. System Organization

### Project Structure (Content Browser)
```
/SteelCathedral
├── /Blueprints
│   ├── /Mech
│   │   ├── BP_PlayerMechPawn
│   │   ├── BP_MechMovementComponent
│   │   ├── BP_WeaponSystemsComponent
│   │   ├── BP_ReactorSystemComponent
│   │   ├── BP_DamageManagementComponent
│   ├── /PlayerStations
│   │   ├── BP_PilotStation
│   │   ├── BP_GunnerStation
│   │   ├── BP_TechnicianCharacter
│   ├── /Enemies
│   │   ├── BP_EnemyMech
│   │   ├── BP_Kaiju
│   │   ├── BP_SwarmUnit
│   │   ├── /AI
│   │   │   ├── BT_EnemyMech (Behavior Tree)
│   │   │   ├── BT_Kaiju
│   │   │   ├── BT_SwarmUnit
│   ├── /Hazards
│   │   ├── BP_FireHazard
│   │   ├── BP_CoolantLeak
│   │   ├── BP_ElectricalArc
│   ├── /Interactables
│   │   ├── BP_Valve
│   │   ├── BP_Switch
│   │   ├── BP_BreakerPanel
│   │   ├── BP_FireExtinguisher
│   ├── /Managers
│   │   ├── BP_CombatStateManager
│   │   ├── BP_HazardManager
│   │   ├── BP_AudioManager
│   │   ├── BP_VFXManager
├── /Maps
│   ├── LVL_UrbanRuins
│   ├── LVL_VolcanicCaldera
│   ├── LVL_FrozenTundra
│   ├── LVL_CanyonNetwork
│   ├── LVL_TestArena (for prototyping)
├── /Materials
│   ├── /Mech
│   │   ├── M_MechHull_Master
│   │   ├── M_DamagedMetal
│   │   ├── M_GlowingReactor
│   ├── /Interior
│   │   ├── M_CockpitGlass
│   │   ├── M_ObservationPort
│   │   ├── M_CrackedGlass
│   │   ├── M_MetalPanel
│   │   ├── M_EmergencyLight
│   ├── /Hazards
│   │   ├── M_Fire
│   │   ├── M_Steam
│   │   ├── M_ElectricalArc
├── /Meshes
│   ├── /Mech
│   │   ├── SM_MechBody
│   │   ├── SM_MechLeg_L / SM_MechLeg_R
│   │   ├── SM_MechArm_L / SM_MechArm_R
│   │   ├── SM_MechHead
│   │   ├── SK_MechFull (Skeletal Mesh)
│   ├── /Interior
│   │   ├── SM_Cockpit
│   │   ├── SM_WeaponsBay
│   │   ├── SM_ReactorCore
│   │   ├── SM_Corridor_Straight / SM_Corridor_Corner
│   │   ├── SM_Ladder
│   │   ├── SM_Catwalk
│   ├── /Enemies
│   │   ├── SK_EnemyMech
│   │   ├── SK_Kaiju
│   │   ├── SK_SwarmUnit
│   ├── /Terrain
│   │   ├── SM_UrbanRubble
│   │   ├── SM_IndustrialDebris
│   │   ├── SM_VolcanicRock
│   │   ├── SM_IceShard
├── /Particles
│   ├── PS_MuzzleFlash
│   ├── PS_Explosion_Large
│   ├── PS_Explosion_Small
│   ├── PS_Steam
│   ├── PS_Sparks
│   ├── PS_Fire
│   ├── PS_CoolantSpray
│   ├── NS_ElectricalArc (Niagara)
├── /Audio
│   ├── /Mech
│   │   ├── SFX_Footstep_Heavy
│   │   ├── SFX_HydraulicHiss
│   │   ├── SFX_ReactorHum
│   │   ├── SFX_AlarmCritical
│   ├── /Weapons
│   │   ├── SFX_CannonFire
│   │   ├── SFX_PlasmaCharge
│   │   ├── SFX_MissileLaunch
│   │   ├── SFX_MinigunLoop
│   ├── /Hazards
│   │   ├── SFX_FireCrackle
│   │   ├── SFX_CoolantLeak
│   │   ├── SFX_ElectricalBuzz
│   ├── /Ambient
│   │   ├── AMB_InteriorCreaking
│   │   ├── AMB_BattlefieldDistant
│   ├── /Music
│   │   ├── MUS_Launch
│   │   ├── MUS_CombatLow
│   │   ├── MUS_CombatHigh
│   │   ├── MUS_Crisis
│   │   ├── MUS_Victory
│   │   ├── MUS_Defeat
├── /UI
│   ├── /Widgets
│   │   ├── WBP_PilotHUD
│   │   ├── WBP_GunnerHUD
│   │   ├── WBP_TechnicianHUD
│   │   ├── WBP_InteractionPrompt
│   │   ├── WBP_MissionBriefing
│   │   ├── WBP_MissionResults
├── /Data
│   ├── DT_WeaponStats (Data Table)
│   ├── DT_EnemyStats
│   ├── DT_TerrainTypes
│   ├── DT_HazardBehavior
```

### System Dependency Map
```
PlayerMechPawn
├── MechMovementComponent
│   ├── Reads: Pilot input, terrain data
│   ├── Writes: Position, movement state, balance
│   ├── Notifies: WeaponSystemsComponent (aim stability)
│   ├── Notifies: ReactorSystemComponent (boost heat)
├── WeaponSystemsComponent
│   ├── Reads: Gunner input, aim stability (from Movement)
│   ├── Writes: Heat contribution, ammo count
│   ├── Notifies: ReactorSystemComponent (power draw)
│   ├── Spawns: Projectile actors
├── ReactorSystemComponent
│   ├── Reads: Heat from weapons/movement, Technician vent actions
│   ├── Writes: Total heat, available power
│   ├── Notifies: All components (power levels)
│   ├── Notifies: HazardManager (overheating triggers fires)
├── DamageManagementComponent
│   ├── Reads: Incoming damage events (from enemies)
│   ├── Writes: Hull integrity per section
│   ├── Notifies: HazardManager (spawn hazards)
│   ├── Notifies: WeaponSystemsComponent (disable weapons)

InteriorEnvironmentActor
├── Contains: Hazard actors (fire, leaks, arcs)
├── Reads: Mech position/rotation (stays synchronized)
├── Listens: DamageManagementComponent (spawn points)

TechnicianCharacter
├── InteractionComponent
│   ├── Reads: Interactable proximity
│   ├── Writes: Interaction state (in-use, completed)
│   ├── Notifies: ReactorSystemComponent (vent action)
│   ├── Notifies: HazardManager (hazard cleared)

CombatStateManager
├── Reads: Enemy health, reactor state, mission timer
├── Writes: Phase state (Launch, Combat, Crisis, Resolution)
├── Notifies: AudioManager (music intensity)
├── Spawns: Enemy actors
├── Listens: PlayerMechPawn (meltdown), EnemyActors (destroyed)

HazardManager
├── Reads: Damage events, reactor overheating
├── Spawns: FireHazard, CoolantLeak, ElectricalArc actors
├── Writes: Hazard positions, spread state
├── Listens: TechnicianCharacter (extinguished events)

AudioManager
├── Reads: Movement state, weapon fire, hazard spawns, combat phase
├── Plays: Localized SFX, alarms, music layers
├── Listens: All major systems for audio cues

VFXManager
├── Reads: Weapon fire, damage events, hazard states
├── Spawns: Particle effects, Niagara systems
├── Listens: ReactorSystemComponent (reactor glow intensity)
```

### Data Flow Example: "Enemy Missile Hits Mech Arm"

1. Enemy AI fires missile → Spawns ProjectileActor
2. ProjectileActor collides with PlayerMechPawn's right arm collision volume
3. DamageManagementComponent receives OnHit event:
   - Identifies hit location: EMechSection::Arm_Right
   - Reduces HullIntegrityMap[Arm_Right] by missile damage value
   - Checks if integrity < critical threshold → Yes
4. DamageManagementComponent broadcasts OnCriticalDamage:
   - Disables right arm minigun (adds to DisabledSystems)
   - Notifies WeaponSystemsComponent → Minigun entry marked offline
   - Seals corridor adjacent to arm section
5. HazardManager receives OnCriticalDamage:
   - Spawns ElectricalArcActor at arm junction point inside interior
6. ElectricalArcActor activates:
   - Niagara sparks play
   - Audio buzz loops
   - Reduces power allocation by 5%
7. ReactorSystemComponent recalculates power:
   - Total available power drops
   - Broadcasts OnPowerReduced
8. WeaponSystemsComponent hears power reduction:
   - Slows plasma array charge rate
   - Displays warning on Gunner's HUD
9. AudioManager plays localized explosion SFX at arm location
10. VFXManager spawns exterior explosion particle effect visible through Gunner's observation port
11. TechnicianCharacter sees:
    - Visual: Sparks showering from ceiling in corridor
    - Audio: Alarm blare + crackling electricity
    - UI: System status panel shows red light for "Arm_Right"
12. Technician navigates to electrical panel, interacts with breaker:
    - InteractionComponent processes input
    - Notifies ElectricalArcActor → Hazard clears
    - DamageManagementComponent logs repair (partial integrity restored)
    - Power allocation returns to normal (minus the disabled minigun)

---

## 9. Implementation Phases

### Phase 1: Core Mech & Movement (Weeks 1-3)
**Goal:** Functional mech that walks and turns.

**Tasks:**
- Build PlayerMechPawn with basic skeletal mesh
- Implement MechMovementComponent (walking, turning, bracing)
- Create test arena map with flat terrain
- Add Pilot station with input handling
- Implement balance system and stumble recovery
- Add footstep audio and camera shake

**Deliverable:** Single-player Pilot can walk the mech around a test environment with responsive controls and weight feedback.

### Phase 2: Interior Environment (Weeks 4-6)
**Goal:** Build the interior space and connect it to mech exterior.

**Tasks:**
- Model cockpit, weapons bay, reactor room, corridors
- Place windows and observation ports
- Synchronize interior position/rotation with mech exterior
- Implement Technician character controller
- Add basic interactables (levers, switches)
- Place diegetic UI elements (gauges, panels)

**Deliverable:** Players can move between stations inside the mech, see the exterior through windows, and interact with objects.

### Phase 3: Weapons & Combat (Weeks 7-9)
**Goal:** Gunner can fire weapons; enemy takes damage.

**Tasks:**
- Implement WeaponSystemsComponent
- Add Gunner station with targeting UI
- Create weapon types: kinetic cannons, plasma, missiles, miniguns
- Build basic enemy mech with health
- Implement projectile actors and hit detection
- Add muzzle flash VFX and weapon audio
- Sync aim stability with Pilot movement state

**Deliverable:** Gunner can target and destroy a stationary enemy mech with multiple weapon types.

### Phase 4: Reactor & Heat Management (Weeks 10-12)
**Goal:** Heat and power systems fully functional.

**Tasks:**
- Implement ReactorSystemComponent
- Add heat generation from weapons and movement
- Create manual venting interaction for Technician
- Build reactor room with animated core and particle effects
- Implement power distribution and rerouting
- Add overheating warnings and critical failure state
- Balance heat/power values for gameplay pacing

**Deliverable:** Firing weapons generates heat, Technician must vent manually, failure to manage heat causes meltdown.

### Phase 5: Damage & Hazards (Weeks 13-15)
**Goal:** Exterior damage creates interior chaos.

**Tasks:**
- Implement DamageManagementComponent
- Create hazard actors: fire, coolant leak, electrical arc
- Build HazardManager with spawn logic
- Add hazard VFX, audio, and spread behavior
- Implement Technician tools: extinguisher, sealant, breaker isolation
- Connect exterior hit locations to interior compartments

**Deliverable:** Enemy hits spawn interior fires/leaks; Technician can extinguish and repair them.

### Phase 6: Enemy AI & Terrain (Weeks 16-18)
**Goal:** Enemies behave tactically; terrain matters.

**Tasks:**
- Build behavior trees for EnemyMech, Kaiju, SwarmUnit
- Implement cover usage, flanking, retreat behaviors
- Create terrain types: urban ruins, volcanic, tundra, canyon
- Add terrain effects on movement (ice slippage, lava heat, rubble slowdown)
- Populate maps with destructible/interactive environment objects

**Deliverable:** Enemies use terrain for cover and tactics; Pilot must adapt movement to terrain type.

### Phase 7: Windows & Scale Visualization (Weeks 19-21)
**Goal:** Windows show combat at intimidating scale.

**Tasks:**
- Model large windscreen and observation ports with proper glass materials
- Implement crack propagation on glass when hit
- Position external cameras and route feeds to interior screens
- Add projectile fly-by effects visible through windows
- Tune enemy size and animation to feel massive when viewed through windows
- Implement acid/fire obscuring glass visibility

**Deliverable:** Players see enemies and combat through massive windows; impacts crack glass and obscure vision.

### Phase 8: Polish & Multiplayer (Weeks 22-26)
**Goal:** Stable three-player networked experience.

**Tasks:**
- Implement replication for all core components
- Test and optimize networking for 60Hz movement/input
- Add voice chat integration
- Tune audio mix for spatial clarity
- Balance heat, power, damage, and hazard values
- Create tutorial mission explaining each role
- Build multiple combat scenarios with varied enemies and terrain

**Deliverable:** Three players can cooperate smoothly in a networked session with stable performance and balanced gameplay.

### Phase 9: Content & Scenarios (Weeks 27-30)
**Goal:** Multiple missions with narrative structure.

**Tasks:**
- Write mission briefs and objectives
- Create 5-6 distinct combat encounters
- Add pre-combat launch sequences
- Implement post-combat results screens
- Design progression system (unlockable weapon types, mech upgrades)
- Add environmental storytelling (graffiti in corridors, mission logs)

**Deliverable:** Full campaign with varied missions, progression, and replayability.

---

## Conclusion

This design document provides the complete technical foundation for building Steel Cathedral in Unreal Engine 5. The system architecture is modular, allowing individual components to be developed and tested independently before integration. The emphasis on physicality, spatial awareness, and coordination ensures that every player role feels essential and interconnected.

### Key Takeaways:
- **Actors:** PlayerMechPawn, InteriorEnvironment, Stations, Enemies, Hazards
- **Components:** Movement, Weapons, Reactor, Damage, Interaction
- **Managers:** CombatState, Audio, VFX, Hazard
- **Systems:** ~15-20 major actors/components, ~30-40 supporting actors (hazards, interactables)
- **Project Structure:** Organized by function (Blueprints, Maps, Materials, Audio, etc.)
- **Implementation:** 30-week phased approach, from core movement to full multiplayer

**Next step:** Prototype Phase 1 (Core Mech & Movement) to validate control feel and scale.
