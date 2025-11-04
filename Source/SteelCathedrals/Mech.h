// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Mech.generated.h"

// Forward declarations
class UMechMovementComponent;
class UReactorSystemComponent;
class UWeaponSystemComponent;
class UDamageManagementComponent;
class UCameraComponent;
class USkeletalMeshComponent;
class UCapsuleComponent;
class UProceduralMechGeometry;
class AProceduralInteriorGeometry;
class AMechStation;

/**
 * Main mech pawn (supports up to 3 crew members in multiplayer)
 * HULL: Exterior mech visual, damage states, weapons
 * INTERIOR: Cockpit environment, stations, hazards
 * EXTERIOR: Handled by game world, terrain, other mechs
 */
UCLASS()
class STEELCATHEDRALS_API AMech : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMech();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Setup replication (multiplayer support for 3 crew members)
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Handle damage
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// ============================================================
	// Components
	// ============================================================

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* MechRoot;

	/** HULL: Mech exterior visual (skeletal or static mesh) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Hull")
	USkeletalMeshComponent* MechMesh;

	/** HULL: Collision capsule */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Hull")
	UCapsuleComponent* MechCapsule;

	/** HULL: Movement component handles all locomotion */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Hull")
	UMechMovementComponent* MechMovement;

	/** HULL: Reactor system handles heat and power */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Hull")
	UReactorSystemComponent* ReactorSystem;

	/** HULL: Weapon system handles all weapons */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Hull")
	UWeaponSystemComponent* WeaponSystem;

	/** HULL: Damage management handles hull integrity and interior hazards */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Hull")
	UDamageManagementComponent* DamageManagement;

	/** INTERIOR: First-person camera (cockpit view) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Interior")
	UCameraComponent* CockpitCamera;

	/** HULL: Procedural geometry generator for mech exterior */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Hull")
	UProceduralMechGeometry* ProceduralGeometry;

	/** INTERIOR: Interior environment actor reference (spawned at runtime) */
	UPROPERTY(BlueprintReadOnly, Category = "Components|Interior")
	AProceduralInteriorGeometry* InteriorEnvironment;

	// ============================================================
	// Mech Specifications
	// ============================================================

	/** Height of the mech in cm (default: 4200cm = 42 meters) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mech|Specs")
	float MechHeight = 4200.0f;

	/** Mass of the mech in kg */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mech|Specs")
	float MechMass = 850000.0f;

	/** Center of mass offset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mech|Specs")
	FVector CenterOfMass = FVector(0, 0, -1000);

	// ============================================================
	// State (Replicated for 3-player multiplayer)
	// ============================================================

	/** Current mech state */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Mech|State")
	FString CurrentMechState = "Active";

	// ============================================================
	// Input Handlers (Can be called from Blueprint or Enhanced Input)
	// ============================================================

	/** Handle movement input (forward/backward, left/right) */
	UFUNCTION(BlueprintCallable, Category = "Mech|Input")
	void HandleMoveInput(FVector2D MovementVector);

	/** Handle look/turn input */
	UFUNCTION(BlueprintCallable, Category = "Mech|Input")
	void HandleTurnInput(float TurnValue);

	/** Handle brace toggle */
	UFUNCTION(BlueprintCallable, Category = "Mech|Input")
	void HandleBracePressed();

	/** Handle boost activation */
	UFUNCTION(BlueprintCallable, Category = "Mech|Input")
	void HandleBoostPressed();

	/** Register the pilot station currently controlling this mech */
	void SetActivePilotStation(AMechStation* Station);

	/** Clear the active pilot station reference if it matches */
	void ClearActivePilotStation(AMechStation* Station);

	// ============================================================
	// Accessors
	// ============================================================

	/** Get the interior environment */
	UFUNCTION(BlueprintCallable, Category = "Mech|Interior")
	AProceduralInteriorGeometry* GetInteriorEnvironment() const { return InteriorEnvironment; }

	// ============================================================
	// Status Queries
	// ============================================================

	/** Get current heat percentage (will be implemented with Reactor component) */
	UFUNCTION(BlueprintPure, Category = "Mech|Status")
	float GetHeatPercentage() const;

	/** Get current power percentage (will be implemented with Reactor component) */
	UFUNCTION(BlueprintPure, Category = "Mech|Status")
	float GetPowerPercentage() const;

	/** Check if a specific system is operational */
	UFUNCTION(BlueprintPure, Category = "Mech|Status")
	bool IsSystemOperational(const FString& SystemName) const;

	// ============================================================
	// Events
	// ============================================================

	/** Called when mech takes damage */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mech|Events")
	void OnMechDamaged(float DamageAmount, FVector HitLocation);

	/** Called when mech state changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mech|Events")
	void OnMechStateChanged(const FString& NewState);

protected:
	// Internal state
	bool bIsBraced = false;

	// Input values (for combining axes)
	float ForwardInputValue = 0.0f;
	float RightInputValue = 0.0f;

	// ============================================================
	// Internal Input Handlers
	// ============================================================

	void InputMoveForward(float Value);
	void InputMoveRight(float Value);
	void UpdateMovementInput();
	void InputTurn(float Value);
	void InputLookUp(float Value);
	void InputFireWeapon();
	void HandleInteractPressed();

	/** Active pilot station reference (used for exiting while controlling the mech) */
	UPROPERTY()
	AMechStation* ActivePilotStation = nullptr;
};
