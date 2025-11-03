# Steel Cathedral - Technical Design Document (TDD)
## Version 1.0 | Unreal Engine 5 Implementation Reference

---

## Table of Contents

1. [System Architecture Overview](#system-architecture-overview)
2. [Core Actor Specifications](#core-actor-specifications)
3. [Component Specifications](#component-specifications)
4. [Manager Subsystems](#manager-subsystems)
5. [Data Structures & Enums](#data-structures--enums)
6. [Network Architecture](#network-architecture)
7. [Performance Requirements](#performance-requirements)
8. [Build Pipeline & Configuration](#build-pipeline--configuration)
9. [Testing Strategy](#testing-strategy)
10. [Development Standards](#development-standards)

---

## 1. System Architecture Overview

### 1.1 High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    GAME INSTANCE LAYER                       │
│  ┌────────────────┐  ┌────────────────┐  ┌───────────────┐ │
│  │ CombatState    │  │ Audio Manager  │  │  VFX Manager  │ │
│  │   Manager      │  │   Subsystem    │  │   Subsystem   │ │
│  └────────────────┘  └────────────────┘  └───────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      WORLD ACTOR LAYER                       │
│  ┌─────────────────────────────────────────────────────┐    │
│  │           PlayerMechPawn (Primary Actor)             │    │
│  │  ┌──────────────┐  ┌──────────────┐  ┌───────────┐ │    │
│  │  │  Movement    │  │   Weapons    │  │  Reactor  │ │    │
│  │  │  Component   │  │  Component   │  │ Component │ │    │
│  │  └──────────────┘  └──────────────┘  └───────────┘ │    │
│  │  ┌──────────────────────────────────────────────┐   │    │
│  │  │      Damage Management Component             │   │    │
│  │  └──────────────────────────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────┘    │
│                              ↓                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │      InteriorEnvironmentActor (Nested)              │    │
│  │  ┌────────────┐  ┌────────────┐  ┌──────────────┐  │    │
│  │  │   Pilot    │  │   Gunner   │  │ Technician   │  │    │
│  │  │  Station   │  │  Station   │  │  Character   │  │    │
│  │  └────────────┘  └────────────┘  └──────────────┘  │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Hazard Manager Actor                    │    │
│  │  ┌──────────┐  ┌──────────┐  ┌─────────────────┐   │    │
│  │  │   Fire   │  │ Coolant  │  │  Electrical     │   │    │
│  │  │  Hazard  │  │   Leak   │  │    Arc          │   │    │
│  │  └──────────┘  └──────────┘  └─────────────────┘   │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              Enemy Actors                            │    │
│  │  ┌──────────┐  ┌──────────┐  ┌─────────────────┐   │    │
│  │  │  Enemy   │  │  Kaiju   │  │  Swarm Unit     │   │    │
│  │  │   Mech   │  │          │  │                 │   │    │
│  │  └──────────┘  └──────────┘  └─────────────────┘   │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 Component Communication Flow

```
Player Input → Station Actor → Component → Mech State Change →
Broadcast Event → Other Components React → Visual/Audio Feedback
```

**Example Flow: Pilot Moves Mech**
```
1. Pilot Input (W key)
2. PilotStationActor captures input
3. Sends command to MechMovementComponent
4. MechMovementComponent updates:
   - CurrentSpeed
   - BalanceValue
   - MovementState = Walking
5. Broadcasts OnMovementStateChanged event
6. WeaponSystemsComponent receives event:
   - Updates AimStabilityModifier
   - Increases reticle bloom
7. ReactorSystemComponent receives event:
   - Increases heat generation rate
8. AudioManager plays footstep sound
9. VFXManager spawns dust particles at feet
```

### 1.3 Replication Authority Model

| System | Authority | Replicated Variables | Update Frequency |
|--------|-----------|---------------------|------------------|
| MechMovementComponent | Pilot Client | Position, Rotation, MovementState | 60 Hz |
| WeaponSystemsComponent | Gunner Client | TargetRotation, FiringState | 60 Hz |
| ReactorSystemComponent | Server | Heat, Power, VentingState | 20 Hz |
| DamageManagementComponent | Server | HullIntegrity, ActiveHazards | On Change |
| InteractionComponent | Technician Client | InteractionState | On Change |

---

## 2. Core Actor Specifications

### 2.1 PlayerMechPawn

**File:** `/Blueprints/Mech/BP_PlayerMechPawn.uasset`

**Parent Class:** `APawn`

**Components:**
- `USceneComponent` - RootComponent
- `USkeletalMeshComponent` - MechMesh
- `UMechMovementComponent` - MovementComponent (Custom)
- `UWeaponSystemsComponent` - WeaponSystems (Custom)
- `UReactorSystemComponent` - ReactorSystem (Custom)
- `UDamageManagementComponent` - DamageManagement (Custom)
- Multiple `UCapsuleComponent` - Collision volumes per body section
- `UAudioComponent` - EngineAmbient, HydraulicLoop, ReactorHum

**Key Properties:**
```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mech Specs")
float MechHeight = 4200.0f; // 42 meters in cm

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mech Specs")
float MechMass = 850000.0f; // kg

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mech Specs")
FVector CenterOfMass = FVector(0, 0, -1000);

UPROPERTY(Replicated)
EMechState CurrentMechState;

UPROPERTY(Replicated)
TArray<AActor*> OccupiedStations;
```

**Key Functions:**
```cpp
// Called when mech takes damage from external source
UFUNCTION()
void OnMechSectionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                      const FHitResult& Hit);

// Server RPC for station occupation
UFUNCTION(Server, Reliable)
void Server_OccupyStation(APlayerController* Player, EStationType StationType);

// Get current total heat percentage
UFUNCTION(BlueprintPure, Category = "Mech|Status")
float GetHeatPercentage() const;

// Get current power percentage
UFUNCTION(BlueprintPure, Category = "Mech|Status")
float GetPowerPercentage() const;

// Check if specific system is operational
UFUNCTION(BlueprintPure, Category = "Mech|Status")
bool IsSystemOperational(ESystemType SystemType) const;
```

**Replication:**
```cpp
void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerMechPawn, CurrentMechState);
    DOREPLIFETIME(APlayerMechPawn, OccupiedStations);
}
```

---

### 2.2 InteriorEnvironmentActor

**File:** `/Blueprints/Mech/BP_InteriorEnvironment.uasset`

**Parent Class:** `AActor`

**Components:**
- `USceneComponent` - RootComponent
- Multiple `UStaticMeshComponent` - Compartment meshes
- Multiple `UPointLightComponent` - Interior lighting
- Multiple `UBoxComponent` - Hazard spawn volumes
- Multiple `USceneCaptureComponent2D` - External camera feeds

**Key Properties:**
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Interior|References")
TMap<ECompartmentType, UStaticMeshComponent*> CompartmentMeshes;

UPROPERTY(EditDefaultsOnly, Category = "Interior|Hazards")
TMap<ECompartmentType, UBoxComponent*> HazardSpawnVolumes;

UPROPERTY(EditDefaultsOnly, Category = "Interior|Lighting")
TArray<UPointLightComponent*> EmergencyLights;

UPROPERTY(EditDefaultsOnly, Category = "Interior|Cameras")
TMap<ECameraLocation, USceneCaptureComponent2D*> ExternalCameras;

UPROPERTY(BlueprintReadOnly, Category = "Interior|State")
TArray<AHazardActor*> ActiveHazards;
```

**Key Functions:**
```cpp
// Synchronize interior transform with mech exterior
UFUNCTION()
void UpdateInteriorTransform(const FTransform& MechTransform);

// Spawn hazard at specific compartment
UFUNCTION(BlueprintCallable, Category = "Interior|Hazards")
void SpawnHazardInCompartment(ECompartmentType Compartment,
                               TSubclassOf<AHazardActor> HazardClass);

// Apply screen shake to all interior players
UFUNCTION(NetMulticast, Reliable)
void Multicast_ApplyInteriorShake(float Intensity, FVector Epicenter);

// Toggle emergency lighting
UFUNCTION(BlueprintCallable, Category = "Interior|Lighting")
void SetEmergencyLightingActive(bool bActive);
```

---

### 2.3 PilotStationActor

**File:** `/Blueprints/PlayerStations/BP_PilotStation.uasset`

**Parent Class:** `AActor` (implements `IInteractable`)

**Components:**
- `USceneComponent` - RootComponent
- `UStaticMeshComponent` - StationMesh
- `UCameraComponent` - PilotCamera
- `UWidgetComponent` - DiegeticUI_Gauges
- `UBoxComponent` - InteractionVolume
- `UInteractionComponent` - InteractionHandler (Custom)

**Key Properties:**
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Station|Input")
UInputMappingContext* PilotInputContext;

UPROPERTY(EditDefaultsOnly, Category = "Station|Input")
TMap<FName, UInputAction*> PilotActions;

UPROPERTY(BlueprintReadOnly, Category = "Station|Occupation")
APlayerController* OccupyingPlayer;

UPROPERTY(EditDefaultsOnly, Category = "Station|UI")
TSubclassOf<UUserWidget> PilotHUDClass;

UPROPERTY()
UUserWidget* PilotHUDInstance;
```

**Key Functions:**
```cpp
// Player enters station
UFUNCTION(BlueprintCallable, Category = "Station")
void OccupyStation(APlayerController* Player);

// Player exits station
UFUNCTION(BlueprintCallable, Category = "Station")
void VacateStation();

// Process movement input
UFUNCTION()
void OnMoveForward(const FInputActionValue& Value);

UFUNCTION()
void OnTurn(const FInputActionValue& Value);

UFUNCTION()
void OnBracePressed();

UFUNCTION()
void OnBoostPressed();

// Update diegetic UI elements
UFUNCTION(BlueprintImplementableEvent, Category = "Station|UI")
void UpdateSpeedGauge(float Speed);

UFUNCTION(BlueprintImplementableEvent, Category = "Station|UI")
void UpdateBalanceIndicator(float Balance);
```

---

### 2.4 GunnerStationActor

**File:** `/Blueprints/PlayerStations/BP_GunnerStation.uasset`

**Parent Class:** `AActor` (implements `IInteractable`)

**Components:**
- Similar structure to PilotStationActor
- Additional `USpringArmComponent` for targeting camera control

**Key Properties:**
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Station|Targeting")
float BaseAimSensitivity = 1.0f;

UPROPERTY(EditDefaultsOnly, Category = "Station|Targeting")
float MinAimStability = 0.3f;

UPROPERTY(EditDefaultsOnly, Category = "Station|Targeting")
float MaxAimStability = 1.0f;

UPROPERTY(BlueprintReadOnly, Category = "Station|State")
float CurrentAimStability = 1.0f;

UPROPERTY(BlueprintReadOnly, Category = "Station|State")
EWeaponType SelectedWeapon;
```

**Key Functions:**
```cpp
// Process aiming input
UFUNCTION()
void OnAim(const FInputActionValue& Value);

// Process weapon firing
UFUNCTION()
void OnFirePressed();

UFUNCTION()
void OnFireReleased();

// Weapon selection
UFUNCTION()
void OnSelectWeapon(EWeaponType WeaponType);

// Update targeting reticle based on stability
UFUNCTION(BlueprintImplementableEvent, Category = "Station|UI")
void UpdateReticleBloom(float Stability);

// Display weapon status
UFUNCTION(BlueprintImplementableEvent, Category = "Station|UI")
void UpdateWeaponStatus(EWeaponType Weapon, int32 Ammo, float Heat);
```

---

### 2.5 TechnicianCharacter

**File:** `/Blueprints/PlayerStations/BP_TechnicianCharacter.uasset`

**Parent Class:** `ACharacter`

**Components:**
- Standard ACharacter components (Capsule, Mesh, Camera, Movement)
- `UInteractionComponent` - InteractionHandler (Custom)
- `UInventoryComponent` - ToolInventory (Custom)

**Key Properties:**
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Technician|Tools")
TMap<EToolType, TSubclassOf<AActor>> ToolClasses;

UPROPERTY(BlueprintReadOnly, Category = "Technician|State")
EToolType CurrentTool;

UPROPERTY(EditDefaultsOnly, Category = "Technician|Movement")
float InteriorWalkSpeed = 300.0f;

UPROPERTY(EditDefaultsOnly, Category = "Technician|Movement")
float InteriorSprintSpeed = 600.0f;

UPROPERTY(BlueprintReadOnly, Category = "Technician|State")
bool bIsHoldingTool;
```

**Key Functions:**
```cpp
// Tool management
UFUNCTION(BlueprintCallable, Category = "Technician|Tools")
void EquipTool(EToolType ToolType);

UFUNCTION(BlueprintCallable, Category = "Technician|Tools")
void UnequipTool();

// Interaction
UFUNCTION()
void OnInteractPressed();

UFUNCTION()
void OnInteractReleased();

// Server RPC for interaction
UFUNCTION(Server, Reliable)
void Server_PerformInteraction(AActor* InteractableActor, EToolType UsedTool);

// Tool usage
UFUNCTION(BlueprintCallable, Category = "Technician|Tools")
void UseFireExtinguisher(AFireHazardActor* Fire);

UFUNCTION(BlueprintCallable, Category = "Technician|Tools")
void UseSealant(ACoolantLeakActor* Leak);

UFUNCTION(BlueprintCallable, Category = "Technician|Tools")
void IsolateBreaker(AElectricalArcActor* Arc);
```

---

## 3. Component Specifications

### 3.1 MechMovementComponent

**File:** `/Blueprints/Mech/BP_MechMovementComponent.uasset`

**Parent Class:** `UActorComponent`

**Key Properties:**
```cpp
// Movement parameters
UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed")
float MaxWalkSpeed = 800.0f; // cm/s

UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed")
float MaxRunSpeed = 1600.0f;

UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed")
float BoostSpeed = 3000.0f;

UPROPERTY(EditDefaultsOnly, Category = "Movement|Rotation")
float TurnRate = 30.0f; // degrees/s

UPROPERTY(EditDefaultsOnly, Category = "Movement|Balance")
float BalanceRecoveryRate = 2.0f; // units/s

UPROPERTY(EditDefaultsOnly, Category = "Movement|Balance")
float StumbleThreshold = -0.7f;

// Current state
UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement|State")
float CurrentSpeed;

UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement|State")
float BalanceValue; // -1.0 to 1.0

UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement|State")
EMoveState MovementState;

UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
ETerrainType CurrentTerrain;

// Boost parameters
UPROPERTY(EditDefaultsOnly, Category = "Movement|Boost")
float BoostDuration = 1.0f;

UPROPERTY(EditDefaultsOnly, Category = "Movement|Boost")
float BoostCooldown = 5.0f;

UPROPERTY(EditDefaultsOnly, Category = "Movement|Boost")
float BoostHeatGeneration = 25.0f;

UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
float BoostCooldownRemaining;
```

**Key Functions:**
```cpp
// Input processing
UFUNCTION(BlueprintCallable, Category = "Movement")
void ProcessMoveInput(FVector2D InputVector);

UFUNCTION(BlueprintCallable, Category = "Movement")
void ProcessTurnInput(float TurnInput);

UFUNCTION(BlueprintCallable, Category = "Movement")
void SetBraced(bool bBraced);

UFUNCTION(BlueprintCallable, Category = "Movement")
void ActivateBoost(FVector Direction);

// State queries
UFUNCTION(BlueprintPure, Category = "Movement")
bool IsBraced() const;

UFUNCTION(BlueprintPure, Category = "Movement")
bool CanBoost() const;

UFUNCTION(BlueprintPure, Category = "Movement")
float GetAimStabilityModifier() const;

// Terrain interaction
UFUNCTION()
void UpdateTerrainType(ETerrainType NewTerrain);

UFUNCTION(BlueprintPure, Category = "Movement")
float GetTerrainSpeedModifier() const;

// Balance management
UFUNCTION()
void ApplyBalanceDisruption(float Magnitude, FVector Direction);

UFUNCTION()
void UpdateBalance(float DeltaTime);
```

**Events:**
```cpp
// Broadcast to other systems
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, EMoveState, NewState);
UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
FOnMovementStateChanged OnMovementStateChanged;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFootstep);
UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
FOnFootstep OnFootstep;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBalanceLost);
UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
FOnBalanceLost OnBalanceLost;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoostActivated, float, HeatGenerated);
UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
FOnBoostActivated OnBoostActivated;
```

**Tick Function:**
```cpp
virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                          FActorComponentTickFunction* ThisTickFunction) override
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update balance
    UpdateBalance(DeltaTime);

    // Update boost cooldown
    if (BoostCooldownRemaining > 0)
    {
        BoostCooldownRemaining -= DeltaTime;
    }

    // Apply terrain modifiers
    float TerrainModifier = GetTerrainSpeedModifier();

    // Update mech position (simplified)
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector MovementDirection = GetOwner()->GetActorForwardVector();
    FVector NewLocation = CurrentLocation + (MovementDirection * CurrentSpeed * TerrainModifier * DeltaTime);

    GetOwner()->SetActorLocation(NewLocation);
}
```

---

### 3.2 WeaponSystemsComponent

**File:** `/Blueprints/Mech/BP_WeaponSystemsComponent.uasset`

**Parent Class:** `UActorComponent`

**Key Properties:**
```cpp
// Weapon definitions
UPROPERTY(EditDefaultsOnly, Category = "Weapons|Configuration")
TArray<FWeaponData> WeaponArray;

UPROPERTY(BlueprintReadOnly, Category = "Weapons|State")
EWeaponType CurrentWeapon;

UPROPERTY(BlueprintReadOnly, Category = "Weapons|State")
float CurrentAimStability = 1.0f;

UPROPERTY(BlueprintReadOnly, Category = "Weapons|State")
bool bIsFiring;

// Targeting
UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapons|Targeting")
FRotator TargetRotation;

UPROPERTY(EditDefaultsOnly, Category = "Weapons|Targeting")
float MaxTargetingRange = 500000.0f; // 5km in cm

UPROPERTY(EditDefaultsOnly, Category = "Weapons|Targeting")
float LockOnTime = 3.0f; // for missiles

UPROPERTY(BlueprintReadOnly, Category = "Weapons|State")
float CurrentLockOnProgress;
```

**Weapon Data Structure:**
```cpp
USTRUCT(BlueprintType)
struct FWeaponData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EWeaponType Type;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName WeaponName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<AProjectileActor> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxAmmo;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentAmmo;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float HeatPerShot;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReloadTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ChargeTime; // 0 for instant fire weapons

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Range;

    UPROPERTY(BlueprintReadOnly)
    float ReloadRemaining;

    UPROPERTY(BlueprintReadOnly)
    bool bIsOverheated;
};
```

**Key Functions:**
```cpp
// Weapon control
UFUNCTION(BlueprintCallable, Category = "Weapons")
void SelectWeapon(EWeaponType WeaponType);

UFUNCTION(BlueprintCallable, Category = "Weapons")
void StartFiring();

UFUNCTION(BlueprintCallable, Category = "Weapons")
void StopFiring();

UFUNCTION(BlueprintCallable, Category = "Weapons")
void UpdateTargetRotation(FRotator NewRotation);

// Firing logic
UFUNCTION(Server, Reliable)
void Server_FireWeapon(EWeaponType WeaponType, FVector MuzzleLocation, FRotator FireRotation);

UFUNCTION()
void SpawnProjectile(EWeaponType WeaponType, FVector Location, FRotator Rotation);

// State queries
UFUNCTION(BlueprintPure, Category = "Weapons")
bool CanFireWeapon(EWeaponType WeaponType) const;

UFUNCTION(BlueprintPure, Category = "Weapons")
FWeaponData GetWeaponData(EWeaponType WeaponType) const;

UFUNCTION(BlueprintPure, Category = "Weapons")
float GetTotalHeatContribution() const;

// External updates
UFUNCTION()
void UpdateAimStability(float NewStability);

UFUNCTION()
void OnPowerLevelChanged(float PowerPercentage);
```

**Events:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponFired, EWeaponType, Weapon, float, HeatGenerated, FVector, Location);
UPROPERTY(BlueprintAssignable, Category = "Weapons|Events")
FOnWeaponFired OnWeaponFired;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponOverheated, EWeaponType, Weapon);
UPROPERTY(BlueprintAssignable, Category = "Weapons|Events")
FOnWeaponOverheated OnWeaponOverheated;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoEmpty, EWeaponType, Weapon);
UPROPERTY(BlueprintAssignable, Category = "Weapons|Events")
FOnAmmoEmpty OnAmmoEmpty;
```

---

### 3.3 ReactorSystemComponent

**File:** `/Blueprints/Mech/BP_ReactorSystemComponent.uasset`

**Parent Class:** `UActorComponent`

**Key Properties:**
```cpp
// Reactor parameters
UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float MaxReactorHeat = 100.0f;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float MaxReactorPower = 100.0f;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float BaseHeatGeneration = 1.0f; // per second

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float PassiveCoolingRate = 0.5f; // per second

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float VentCoolingAmount = 30.0f;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float VentDuration = 10.0f;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float VentPowerReduction = 25.0f; // percentage

// Current state
UPROPERTY(Replicated, BlueprintReadOnly, Category = "Reactor|State")
float ReactorHeat;

UPROPERTY(Replicated, BlueprintReadOnly, Category = "Reactor|State")
float ReactorPower;

UPROPERTY(Replicated, BlueprintReadOnly, Category = "Reactor|State")
bool bVentingInProgress;

UPROPERTY(BlueprintReadOnly, Category = "Reactor|State")
float VentingTimeRemaining;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
TMap<ESystemType, float> BasePowerAllocation;

UPROPERTY(BlueprintReadOnly, Category = "Reactor|State")
TMap<ESystemType, float> CurrentPowerAllocation;

// Heat thresholds
UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float WarningThreshold = 50.0f;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float DangerThreshold = 75.0f;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float CriticalThreshold = 90.0f;

UPROPERTY(EditDefaultsOnly, Category = "Reactor|Configuration")
float MeltdownThreshold = 100.0f;
```

**Key Functions:**
```cpp
// Heat management
UFUNCTION(BlueprintCallable, Category = "Reactor")
void AddHeat(float HeatAmount);

UFUNCTION(BlueprintCallable, Category = "Reactor")
void ReduceHeat(float HeatAmount);

UFUNCTION(BlueprintCallable, Category = "Reactor")
void InitiateVenting();

UFUNCTION(Server, Reliable)
void Server_InitiateVenting();

UFUNCTION()
void CompleteVenting();

// Power management
UFUNCTION(BlueprintCallable, Category = "Reactor")
void ReallocatePower(ESystemType SystemType, float PowerPercentage);

UFUNCTION(BlueprintCallable, Category = "Reactor")
void DivertPowerFromSystem(ESystemType SystemType);

UFUNCTION(BlueprintPure, Category = "Reactor")
float GetAvailablePowerForSystem(ESystemType SystemType) const;

// State queries
UFUNCTION(BlueprintPure, Category = "Reactor")
float GetHeatPercentage() const;

UFUNCTION(BlueprintPure, Category = "Reactor")
float GetPowerPercentage() const;

UFUNCTION(BlueprintPure, Category = "Reactor")
EReactorState GetReactorState() const;

UFUNCTION(BlueprintPure, Category = "Reactor")
bool CanVent() const;

// Environmental modifiers
UFUNCTION()
void ApplyEnvironmentalHeat(float HeatPerSecond);
```

**Events:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeatThresholdReached, EReactorState, NewState);
UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
FOnHeatThresholdReached OnHeatThresholdReached;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVentingStarted);
UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
FOnVentingStarted OnVentingStarted;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVentingCompleted);
UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
FOnVentingCompleted OnVentingCompleted;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeltdownInitiated);
UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
FOnMeltdownInitiated OnMeltdownInitiated;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerReduced, ESystemType, AffectedSystem, float, NewPowerLevel);
UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
FOnPowerReduced OnPowerReduced;
```

**Tick Function:**
```cpp
virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                          FActorComponentTickFunction* ThisTickFunction) override
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Apply base heat generation
    ReactorHeat += BaseHeatGeneration * DeltaTime;

    // Apply passive cooling
    if (!bVentingInProgress)
    {
        ReactorHeat = FMath::Max(0.0f, ReactorHeat - (PassiveCoolingRate * DeltaTime));
    }
    else
    {
        // Active venting
        VentingTimeRemaining -= DeltaTime;
        ReactorHeat = FMath::Max(0.0f, ReactorHeat - (VentCoolingAmount / VentDuration) * DeltaTime);

        if (VentingTimeRemaining <= 0)
        {
            CompleteVenting();
        }
    }

    // Check heat thresholds
    EReactorState NewState = GetReactorState();
    if (NewState != PreviousReactorState)
    {
        OnHeatThresholdReached.Broadcast(NewState);
        PreviousReactorState = NewState;
    }

    // Check meltdown
    if (ReactorHeat >= MeltdownThreshold)
    {
        OnMeltdownInitiated.Broadcast();
    }
}
```

---

### 3.4 DamageManagementComponent

**File:** `/Blueprints/Mech/BP_DamageManagementComponent.uasset`

**Parent Class:** `UActorComponent`

**Key Properties:**
```cpp
// Hull integrity
UPROPERTY(EditDefaultsOnly, Category = "Damage|Configuration")
TMap<EMechSection, float> MaxHullIntegrity;

UPROPERTY(Replicated, BlueprintReadOnly, Category = "Damage|State")
TMap<EMechSection, float> CurrentHullIntegrity;

// Hazard spawning
UPROPERTY(EditDefaultsOnly, Category = "Damage|Hazards")
TMap<EDamageType, FHazardSpawnData> HazardSpawnProbabilities;

UPROPERTY(Replicated, BlueprintReadOnly, Category = "Damage|State")
TArray<AHazardActor*> ActiveHazards;

// System status
UPROPERTY(Replicated, BlueprintReadOnly, Category = "Damage|State")
TArray<ESystemType> DisabledSystems;

UPROPERTY(EditDefaultsOnly, Category = "Damage|Configuration")
TMap<EMechSection, TArray<ESystemType>> SectionToSystemMapping;

// Critical damage thresholds
UPROPERTY(EditDefaultsOnly, Category = "Damage|Configuration")
float CriticalDamageThreshold = 0.3f; // 30% integrity remaining
```

**Hazard Spawn Data:**
```cpp
USTRUCT(BlueprintType)
struct FHazardSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<AHazardActor> HazardClass;

    UPROPERTY(EditDefaultsOnly)
    float SpawnProbability; // 0-1

    UPROPERTY(EditDefaultsOnly)
    int32 MaxSimultaneousHazards;
};
```

**Key Functions:**
```cpp
// Damage processing
UFUNCTION()
void ProcessDamage(EMechSection Section, float DamageAmount, EDamageType DamageType,
                   FVector HitLocation, FVector HitDirection);

UFUNCTION(Server, Reliable)
void Server_ProcessDamage(EMechSection Section, float DamageAmount, EDamageType DamageType,
                          FVector HitLocation, FVector HitDirection);

// Hazard management
UFUNCTION()
void SpawnHazard(EMechSection Section, EDamageType DamageType, FVector Location);

UFUNCTION(NetMulticast, Reliable)
void Multicast_SpawnHazard(TSubclassOf<AHazardActor> HazardClass,
                           ECompartmentType Compartment, FVector Location);

UFUNCTION()
void RemoveHazard(AHazardActor* Hazard);

// System management
UFUNCTION()
void DisableSystem(ESystemType SystemType);

UFUNCTION()
void EnableSystem(ESystemType SystemType);

UFUNCTION(BlueprintPure, Category = "Damage")
bool IsSystemOnline(ESystemType SystemType) const;

// Repair
UFUNCTION(BlueprintCallable, Category = "Damage")
void RepairSection(EMechSection Section, float RepairAmount);

// State queries
UFUNCTION(BlueprintPure, Category = "Damage")
float GetSectionIntegrityPercentage(EMechSection Section) const;

UFUNCTION(BlueprintPure, Category = "Damage")
bool IsSectionCritical(EMechSection Section) const;

UFUNCTION(BlueprintPure, Category = "Damage")
TArray<EMechSection> GetCriticalSections() const;
```

**Events:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSectionDamaged, EMechSection, Section,
                                               float, RemainingIntegrity, FVector, HitLocation);
UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
FOnSectionDamaged OnSectionDamaged;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCriticalDamage, EMechSection, Section,
                                             ESystemType, DisabledSystem);
UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
FOnCriticalDamage OnCriticalDamage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHazardSpawned, AHazardActor*, Hazard,
                                             ECompartmentType, Compartment);
UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
FOnHazardSpawned OnHazardSpawned;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardCleared, AHazardActor*, Hazard);
UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
FOnHazardCleared OnHazardCleared;
```

---

## 4. Manager Subsystems

### 4.1 CombatStateManager

**File:** `/Blueprints/Managers/BP_CombatStateManager.uasset`

**Parent Class:** `UGameInstanceSubsystem`

**Key Properties:**
```cpp
UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
ECombatPhase CurrentPhase;

UPROPERTY(BlueprintReadOnly, Category = "Combat|Mission")
FMissionData CurrentMission;

UPROPERTY(BlueprintReadOnly, Category = "Combat|Enemies")
TArray<AActor*> ActiveEnemies;

UPROPERTY(EditDefaultsOnly, Category = "Combat|Configuration")
TMap<ECombatPhase, float> PhaseTimeouts;

UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
float PhaseTimeRemaining;
```

**Mission Data:**
```cpp
USTRUCT(BlueprintType)
struct FMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText MissionName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText MissionBrief;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EMissionObjective ObjectiveType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FEnemySpawnData> EnemySpawns;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName TerrainMap;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MissionTimeout;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AmbientHeatModifier;
};
```

**Key Functions:**
```cpp
// Phase management
UFUNCTION(BlueprintCallable, Category = "Combat")
void StartMission(const FMissionData& MissionData);

UFUNCTION(BlueprintCallable, Category = "Combat")
void AdvancePhase();

UFUNCTION(BlueprintCallable, Category = "Combat")
void SetPhase(ECombatPhase NewPhase);

// Enemy management
UFUNCTION(BlueprintCallable, Category = "Combat|Enemies")
void SpawnEnemy(TSubclassOf<AActor> EnemyClass, FVector Location, FRotator Rotation);

UFUNCTION()
void OnEnemyDestroyed(AActor* Enemy);

UFUNCTION(BlueprintPure, Category = "Combat|Enemies")
int32 GetRemainingEnemyCount() const;

// Win/Lose conditions
UFUNCTION()
void CheckVictoryCondition();

UFUNCTION()
void OnMissionComplete();

UFUNCTION()
void OnMissionFailed(EMissionFailureReason Reason);
```

**Events:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, ECombatPhase, NewPhase);
UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
FOnPhaseChanged OnPhaseChanged;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionComplete);
UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
FOnMissionComplete OnMissionComplete;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, EMissionFailureReason, Reason);
UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
FOnMissionFailed OnMissionFailed;
```

---

### 4.2 HazardManager

**File:** `/Blueprints/Managers/BP_HazardManager.uasset`

**Parent Class:** `AActor`

**Key Properties:**
```cpp
UPROPERTY(BlueprintReadOnly, Category = "Hazards|State")
TArray<AHazardActor*> ActiveHazards;

UPROPERTY(EditDefaultsOnly, Category = "Hazards|Configuration")
TMap<EHazardType, FHazardBehaviorData> HazardBehaviors;

UPROPERTY(EditDefaultsOnly, Category = "Hazards|Configuration")
int32 MaxSimultaneousHazards = 10;

UPROPERTY(EditDefaultsOnly, Category = "Hazards|Spawning")
float MinHazardSpacing = 500.0f; // cm
```

**Hazard Behavior Data:**
```cpp
USTRUCT(BlueprintType)
struct FHazardBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    float SpreadRate; // cm/s

    UPROPERTY(EditDefaultsOnly)
    float SpreadInterval; // seconds between spread attempts

    UPROPERTY(EditDefaultsOnly)
    float MaxSpreadDistance; // cm

    UPROPERTY(EditDefaultsOnly)
    float SystemImpactPerSecond; // damage or heat or power drain

    UPROPERTY(EditDefaultsOnly)
    float LifetimeDuration; // 0 = infinite until cleared
};
```

**Key Functions:**
```cpp
// Hazard lifecycle
UFUNCTION(BlueprintCallable, Category = "Hazards")
AHazardActor* SpawnHazard(TSubclassOf<AHazardActor> HazardClass,
                          ECompartmentType Compartment, FVector Location);

UFUNCTION(BlueprintCallable, Category = "Hazards")
void RemoveHazard(AHazardActor* Hazard);

UFUNCTION()
void OnHazardExtinguished(AHazardActor* Hazard);

// Spread mechanics
UFUNCTION()
void AttemptHazardSpread(AHazardActor* Hazard);

UFUNCTION()
bool CanSpreadToLocation(FVector Location, EHazardType HazardType) const;

// State queries
UFUNCTION(BlueprintPure, Category = "Hazards")
TArray<AHazardActor*> GetHazardsInCompartment(ECompartmentType Compartment) const;

UFUNCTION(BlueprintPure, Category = "Hazards")
int32 GetHazardCount() const;

UFUNCTION(BlueprintPure, Category = "Hazards")
float GetTotalHazardImpact() const;
```

**Events:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHazardSpawned, AHazardActor*, Hazard,
                                             ECompartmentType, Compartment);
UPROPERTY(BlueprintAssignable, Category = "Hazards|Events")
FOnHazardSpawned OnHazardSpawned;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHazardSpread, AHazardActor*, OriginalHazard,
                                             AHazardActor*, NewHazard);
UPROPERTY(BlueprintAssignable, Category = "Hazards|Events")
FOnHazardSpread OnHazardSpread;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardExtinguished, AHazardActor*, Hazard);
UPROPERTY(BlueprintAssignable, Category = "Hazards|Events")
FOnHazardExtinguished OnHazardExtinguished;
```

---

## 5. Data Structures & Enums

### 5.1 Core Enumerations

```cpp
// Mech state
UENUM(BlueprintType)
enum class EMechState : uint8
{
    Inactive,
    Booting,
    Active,
    Critical,
    Disabled
};

// Movement states
UENUM(BlueprintType)
enum class EMoveState : uint8
{
    Idle,
    Walking,
    Running,
    Braced,
    Strafing,
    Boosting,
    Stumbling,
    Fallen
};

// Weapon types
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None,
    KineticCannon_Left,
    KineticCannon_Right,
    PlasmaArray,
    MissilePods,
    Minigun_Left,
    Minigun_Right
};

// System types
UENUM(BlueprintType)
enum class ESystemType : uint8
{
    Movement,
    Weapons,
    Reactor,
    LifeSupport,
    Sensors,
    Communications,
    Hydraulics,
    Auxiliary
};

// Mech sections
UENUM(BlueprintType)
enum class EMechSection : uint8
{
    Head,
    Torso_Center,
    Torso_Left,
    Torso_Right,
    Arm_Left,
    Arm_Right,
    Leg_Left,
    Leg_Right,
    Back
};

// Terrain types
UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    Stable,
    Loose,
    Ice,
    Lava,
    Water,
    Hazardous
};

// Reactor states
UENUM(BlueprintType)
enum class EReactorState : uint8
{
    Normal,
    Warning,
    Danger,
    Critical,
    Meltdown
};

// Damage types
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Kinetic,
    Plasma,
    Explosive,
    Acid,
    Electrical,
    Thermal
};

// Hazard types
UENUM(BlueprintType)
enum class EHazardType : uint8
{
    Fire,
    CoolantLeak,
    ElectricalArc,
    StructuralCollapse,
    Flooding
};

// Compartment types
UENUM(BlueprintType)
enum class ECompartmentType : uint8
{
    Cockpit,
    WeaponsBay,
    ReactorRoom,
    Corridor_Forward,
    Corridor_Mid,
    Corridor_Aft,
    HydraulicRoom,
    AmmoStorage,
    LifeSupport,
    BackupGenerator
};

// Station types
UENUM(BlueprintType)
enum class EStationType : uint8
{
    None,
    Pilot,
    Gunner
};

// Tool types
UENUM(BlueprintType)
enum class EToolType : uint8
{
    None,
    FireExtinguisher,
    Sealant,
    Wrench,
    Multimeter
};

// Combat phases
UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
    PreLaunch,
    Launch,
    Combat,
    Crisis,
    Resolution
};

// Mission objectives
UENUM(BlueprintType)
enum class EMissionObjective : uint8
{
    DestroyAllEnemies,
    SurviveTime,
    DefendLocation,
    ReachLocation
};

// Mission failure reasons
UENUM(BlueprintType)
enum class EMissionFailureReason : uint8
{
    ReactorMeltdown,
    TotalDestruction,
    Timeout,
    ObjectiveFailed
};

// Camera locations
UENUM(BlueprintType)
enum class ECameraLocation : uint8
{
    Front,
    Rear,
    Left,
    Right,
    Top
};
```

### 5.2 Key Data Tables

**Weapon Stats Data Table**
```cpp
// Row structure for DT_WeaponStats
USTRUCT(BlueprintType)
struct FWeaponStatsRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EWeaponType WeaponType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName WeaponName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxAmmo;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float HeatPerShot;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReloadTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ChargeTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Range;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ProjectileSpeed;
};
```

**Enemy Stats Data Table**
```cpp
// Row structure for DT_EnemyStats
USTRUCT(BlueprintType)
struct FEnemyStatsRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName EnemyName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MaxHealth;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MoveSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackCooldown;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<EDamageType> ResistantTo;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<EDamageType> WeakTo;
};
```

---

## 6. Network Architecture

### 6.1 Replication Model

**Authority Distribution:**
- **Server Authority:** Reactor state, damage, hazards, enemy AI
- **Client Authority:** Player inputs, station occupation, local predictions

**Replication Frequency:**
- Critical variables (position, rotation): 60 Hz
- State variables (heat, power, ammo): 20 Hz
- Event-based (damage, hazard spawn): On change

### 6.2 Network Optimization

**Relevancy Rules:**
```cpp
// In PlayerMechPawn
bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
                     const FVector& SrcLocation) const override
{
    // Mech is always relevant to all players in the session
    return true;
}
```

**Variable Replication Conditions:**
```cpp
// In MechMovementComponent
void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // High-frequency, critical for sync
    DOREPLIFETIME_CONDITION(UMechMovementComponent, CurrentSpeed, COND_SimulatedOnly);
    DOREPLIFETIME_CONDITION(UMechMovementComponent, MovementState, COND_SimulatedOnly);

    // Lower-frequency, less critical
    DOREPLIFETIME_CONDITION(UMechMovementComponent, BalanceValue, COND_SkipOwner);
}
```

**Client-Side Prediction:**
```cpp
// In GunnerStationActor
void OnAim(const FInputActionValue& Value)
{
    FVector2D AimInput = Value.Get<FVector2D>();

    // Immediately update local rotation (prediction)
    LocalTargetRotation += FRotator(AimInput.Y, AimInput.X, 0) * AimSensitivity;

    // Send to server
    Server_UpdateTargetRotation(LocalTargetRotation);
}

UFUNCTION(Server, Unreliable)
void Server_UpdateTargetRotation(FRotator NewRotation)
{
    // Server validates and replicates to other clients
    if (WeaponSystemsComponent)
    {
        WeaponSystemsComponent->UpdateTargetRotation(NewRotation);
    }
}
```

### 6.3 Network Security

**Server Validation:**
```cpp
// In ReactorSystemComponent
UFUNCTION(Server, Reliable)
void Server_InitiateVenting()
{
    // Validate venting is possible
    if (!CanVent())
    {
        return; // Reject invalid client request
    }

    // Validate technician is at reactor station
    if (!IsPlayerAtReactorStation(GetOwner()))
    {
        return; // Reject - player not in position
    }

    // Proceed with validated action
    bVentingInProgress = true;
    VentingTimeRemaining = VentDuration;
    OnVentingStarted.Broadcast();
}
```

---

## 7. Performance Requirements

### 7.1 Target Performance Metrics

| Metric | Target | Minimum |
|--------|--------|---------|
| Frame Rate | 90 FPS (VR) / 144 FPS (Desktop) | 60 FPS |
| Network Latency | < 50ms | < 100ms |
| Memory Usage | < 6 GB | < 8 GB |
| VRAM Usage | < 4 GB | < 6 GB |
| Load Time | < 30s | < 60s |

### 7.2 Optimization Strategies

**Level of Detail (LOD):**
- Exterior mech: 4 LOD levels
- Interior meshes: 2 LOD levels (high detail when inside)
- Enemy meshes: 4 LOD levels based on distance

**Culling:**
- Interior compartments occluded when not visible
- Exterior world culled when players inside mech
- Hazard VFX culled based on distance from players

**Particle Budget:**
- Max simultaneous particle systems: 50
- Max particles per system: 1000
- Hazard VFX: 500 particles per hazard

**Audio Occlusion:**
- Interior sounds attenuated by compartment walls
- Exterior sounds filtered when inside mech
- Max simultaneous 3D audio sources: 32

### 7.3 Profiling Checkpoints

**Phase 1-3:** < 16ms frame time on test rig
**Phase 4-6:** < 20ms frame time with full systems
**Phase 7-9:** < 11ms frame time (90 FPS) optimized

---

## 8. Build Pipeline & Configuration

### 8.1 Project Configuration

**DefaultEngine.ini:**
```ini
[/Script/Engine.Engine]
bSmoothFrameRate=True
SmoothedFrameRateRange=(LowerBound=(Type=Inclusive,Value=60.000000),UpperBound=(Type=Exclusive,Value=144.000000))

[/Script/OnlineSubsystemUtils.IpNetDriver]
NetServerMaxTickRate=60
MaxNetTickRate=60
MaxInternetClientRate=25000
MaxClientRate=25000

[/Script/Engine.GameNetworkManager]
TotalNetBandwidth=32000
MaxDynamicBandwidth=20000
MinDynamicBandwidth=10000

[/Script/Engine.Player]
ConfiguredInternetSpeed=25000
ConfiguredLanSpeed=30000

[/Script/Engine.GameEngine]
MaximumLoopIterationCount=1000000

[/Script/UnrealEd.CookerSettings]
bCookOnTheFlyForLaunchOn=False
```

**DefaultGame.ini:**
```ini
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Game/Maps/LVL_TestArena
ServerDefaultMap=/Game/Maps/LVL_TestArena
GlobalDefaultGameMode=/Game/Blueprints/Core/BP_SteelCathedralGameMode

[/Script/SteelCathedral.SteelCathedralGameMode]
MaxPlayers=3
bAllowJoinInProgress=False
bRequireFullTeam=True
```

### 8.2 Build Configurations

**Development Build:**
- Full debug symbols
- Logging enabled
- Cheats enabled
- Network emulation tools accessible

**Shipping Build:**
- Optimizations: Maximum
- Debug symbols: Stripped
- Logging: Errors only
- Cheats: Disabled
- Pak file encryption: Enabled

---

## 9. Testing Strategy

### 9.1 Unit Testing

**Component Tests:**
```cpp
// Example: MechMovementComponent test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMechMovementTest,
    "SteelCathedral.Components.MechMovement.BasicMovement",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMechMovementTest::RunTest(const FString& Parameters)
{
    // Setup
    UWorld* TestWorld = CreateTestWorld();
    APlayerMechPawn* TestMech = SpawnTestMech(TestWorld);
    UMechMovementComponent* Movement = TestMech->FindComponentByClass<UMechMovementComponent>();

    // Test: Forward movement increases speed
    Movement->ProcessMoveInput(FVector2D(1.0f, 0.0f));
    TestWorld->Tick(LEVELTICK_All, 0.016f); // 60 FPS tick

    TestTrue("Speed increased after forward input", Movement->CurrentSpeed > 0);

    // Test: Bracing sets correct state
    Movement->SetBraced(true);
    TestEqual("Movement state is Braced", Movement->MovementState, EMoveState::Braced);
    TestEqual("Speed is zero when braced", Movement->CurrentSpeed, 0.0f);

    // Cleanup
    DestroyTestWorld(TestWorld);

    return true;
}
```

### 9.2 Integration Testing

**System Interaction Tests:**
- Weapon fire → Heat generation → Reactor response
- Damage received → Hazard spawn → Technician repair
- Venting initiation → Power reduction → Weapon cooldown

### 9.3 Multiplayer Testing

**Network Scenarios:**
- 3 players, < 50ms latency (optimal)
- 3 players, 100-150ms latency (degraded)
- 3 players, packet loss 1-5% (stress test)
- Player disconnection/reconnection

**Synchronization Validation:**
- Mech position/rotation matches on all clients
- Heat/power values consistent across clients
- Hazard spawns visible to all players
- Damage state replicated correctly

### 9.4 Performance Testing

**Profiling Tools:**
- Unreal Insights for frame analysis
- Network Profiler for bandwidth usage
- Memory Profiler for leak detection

**Stress Tests:**
- Maximum hazards active (10 simultaneous)
- All weapons firing simultaneously
- Multiple enemies on screen (3+ enemy mechs)
- Extended play sessions (60+ minutes)

---

## 10. Development Standards

### 10.1 Naming Conventions

**Blueprints:**
- Actor: `BP_<Name>` (e.g., `BP_PlayerMechPawn`)
- Component: `BP_<Name>Component` (e.g., `BP_MechMovementComponent`)
- Widget: `WBP_<Name>` (e.g., `WBP_PilotHUD`)

**Assets:**
- Static Mesh: `SM_<Name>` (e.g., `SM_Cockpit`)
- Skeletal Mesh: `SK_<Name>` (e.g., `SK_MechFull`)
- Material: `M_<Name>` (e.g., `M_MechHull_Master`)
- Material Instance: `MI_<Name>` (e.g., `MI_MechHull_Damaged`)
- Texture: `T_<Name>` (e.g., `T_MechHull_Albedo`)
- Particle System: `PS_<Name>` (e.g., `PS_Explosion_Large`)
- Niagara System: `NS_<Name>` (e.g., `NS_ElectricalArc`)
- Sound Cue: `SFX_<Name>` or `MUS_<Name>` (e.g., `SFX_CannonFire`)

**Code:**
- Class: `PascalCase` (e.g., `AMechMovementComponent`)
- Function: `PascalCase` (e.g., `ProcessMoveInput`)
- Variable: `camelCase` with prefix (e.g., `bIsBraced`, `CurrentSpeed`)
- Enum: `EPascalCase` (e.g., `EMoveState`)
- Struct: `FPascalCase` (e.g., `FWeaponData`)

### 10.2 Blueprint Organization

**Graph Layout:**
- Input events → Top-left
- Main logic flow → Left to right
- Helper functions → Collapsed to macros/functions
- Comments for all major sections

**Variable Categories:**
- Configuration (EditDefaultsOnly)
- State (BlueprintReadOnly)
- References (EditInstanceOnly)
- Internal (Private)

### 10.3 Version Control

**Branch Strategy:**
- `main`: Stable, tested builds only
- `develop`: Integration branch for features
- `feature/<name>`: Individual feature development
- `hotfix/<name>`: Critical bug fixes

**Commit Guidelines:**
- Prefix commits: `[Feature]`, `[Fix]`, `[Refactor]`, `[Docs]`
- Reference task IDs in commits
- Keep commits atomic and focused

### 10.4 Code Review Checklist

- [ ] Replication configured correctly
- [ ] Server validation for authority actions
- [ ] Performance profiled (< 0.1ms per tick)
- [ ] Blueprint compile warnings resolved
- [ ] Events properly broadcast/bound
- [ ] Memory leaks checked (no dangling references)
- [ ] Multiplayer tested (3 clients)
- [ ] Edge cases handled (null checks, bounds)

---

## Appendix A: Quick Reference

### Common Event Bindings

```cpp
// In PlayerMechPawn::BeginPlay
void APlayerMechPawn::BeginPlay()
{
    Super::BeginPlay();

    // Bind movement events
    MovementComponent->OnMovementStateChanged.AddDynamic(this, &APlayerMechPawn::OnMovementChanged);
    MovementComponent->OnBoostActivated.AddDynamic(ReactorSystem, &UReactorSystemComponent::AddHeat);

    // Bind weapon events
    WeaponSystems->OnWeaponFired.AddDynamic(ReactorSystem, &UReactorSystemComponent::OnWeaponFired);
    WeaponSystems->OnWeaponFired.AddDynamic(this, &APlayerMechPawn::OnWeaponFiredVisuals);

    // Bind reactor events
    ReactorSystem->OnHeatThresholdReached.AddDynamic(this, &APlayerMechPawn::OnReactorStateChanged);
    ReactorSystem->OnMeltdownInitiated.AddDynamic(this, &APlayerMechPawn::OnMeltdown);

    // Bind damage events
    DamageManagement->OnSectionDamaged.AddDynamic(this, &APlayerMechPawn::OnHullDamaged);
    DamageManagement->OnCriticalDamage.AddDynamic(this, &APlayerMechPawn::OnCriticalDamage);
}
```

### Common RPC Patterns

```cpp
// Client → Server → Multicast pattern
// Example: Weapon firing

// 1. Client calls
UFUNCTION(BlueprintCallable)
void FireWeapon()
{
    // Client prediction (immediate feedback)
    PlayLocalMuzzleFlash();

    // Send to server
    Server_FireWeapon(TargetRotation);
}

// 2. Server validates and processes
UFUNCTION(Server, Reliable)
void Server_FireWeapon(FRotator FireRotation)
{
    if (!CanFire()) return;

    // Server-authoritative logic
    SpawnProjectile(FireRotation);
    ConsumeAmmo();

    // Replicate to all clients
    Multicast_PlayWeaponFireEffects(FireRotation);
}

// 3. All clients see result
UFUNCTION(NetMulticast, Reliable)
void Multicast_PlayWeaponFireEffects(FRotator FireRotation)
{
    PlayMuzzleFlash();
    PlayFireSound();
    SpawnCasingEject();
}
```

---

**Document Version:** 1.0
**Last Updated:** 2025
**Engine Version:** Unreal Engine 5.4+
**Target Platforms:** Windows PC, VR (PCVR)

---

*End of Technical Design Document*
