// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageManagementComponent.generated.h"

class AFireHazardActor;
class ACoolantLeakActor;
class AElectricalArcActor;

/**
 * Mech section types for localized damage
 */
UENUM(BlueprintType)
enum class EMechSection : uint8
{
	Head UMETA(DisplayName = "Head"),
	Torso UMETA(DisplayName = "Torso"),
	LeftArm UMETA(DisplayName = "Left Arm"),
	RightArm UMETA(DisplayName = "Right Arm"),
	LeftLeg UMETA(DisplayName = "Left Leg"),
	RightLeg UMETA(DisplayName = "Right Leg")
};

/**
 * Subsystem types that can fail
 */
UENUM(BlueprintType)
enum class ESubsystemType : uint8
{
	Sensors UMETA(DisplayName = "Sensors"),
	LifeSupport UMETA(DisplayName = "Life Support"),
	Communications UMETA(DisplayName = "Communications"),
	Stabilizers UMETA(DisplayName = "Stabilizers"),
	Actuators UMETA(DisplayName = "Actuators"),
	CoolantPump UMETA(DisplayName = "Coolant Pump"),
	PowerDistribution UMETA(DisplayName = "Power Distribution")
};

/**
 * Damage type for different attack sources
 */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Ballistic UMETA(DisplayName = "Ballistic"),
	Energy UMETA(DisplayName = "Energy"),
	Explosive UMETA(DisplayName = "Explosive"),
	Melee UMETA(DisplayName = "Melee"),
	Environmental UMETA(DisplayName = "Environmental")
};

/**
 * Data for a specific mech section
 */
USTRUCT(BlueprintType)
struct FMechSectionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMechSection Section = EMechSection::Torso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxIntegrity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentIntegrity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ESubsystemType> AffectedSubsystems;

	// Get integrity as percentage
	float GetIntegrityPercent() const
	{
		if (MaxIntegrity <= 0.0f) return 0.0f;
		return FMath::Clamp(CurrentIntegrity / MaxIntegrity, 0.0f, 1.0f);
	}

	// Check if section is critical (<25% integrity)
	bool IsCritical() const
	{
		return GetIntegrityPercent() < 0.25f && !bIsDestroyed;
	}
};

/**
 * Data for a subsystem that can fail
 */
USTRUCT(BlueprintType)
struct FSubsystemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESubsystemType Type = ESubsystemType::Sensors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SubsystemName = "System";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOperational = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EfficiencyPercent = 100.0f; // 0-100, can be damaged but not destroyed

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMechSection LinkedSection = EMechSection::Torso;
};

/**
 * Manages mech damage, hull integrity, and interior hazard spawning
 * Connects exterior damage to interior consequences
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEELCATHEDRALS_API UDamageManagementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageManagementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================
	// Configuration
	// ============================================================

	/** Total hull integrity - sum of all sections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Config")
	float TotalMaxIntegrity = 6000.0f;

	/** Chance to spawn hazard when section takes damage (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Config")
	float HazardSpawnChance = 0.3f;

	/** Minimum damage to trigger hazard spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Config")
	float MinDamageForHazard = 100.0f;

	/** Interior spawn points for hazards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage|Config")
	TArray<FVector> InteriorHazardSpawnPoints;

	// ============================================================
	// Section Data
	// ============================================================

	/** All mech sections with individual integrity */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Damage|State")
	TArray<FMechSectionData> Sections;

	/** All subsystems that can fail */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Damage|State")
	TArray<FSubsystemData> Subsystems;

	// ============================================================
	// Damage Application
	// ============================================================

	/** Apply damage to a specific section */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyDamage(EMechSection Section, float DamageAmount, EDamageType DamageType, FVector HitLocation);

	/** Apply damage to closest section based on world hit location */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyDamageAtLocation(float DamageAmount, EDamageType DamageType, FVector WorldHitLocation);

	/** Repair a specific section */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void RepairSection(EMechSection Section, float RepairAmount);

	/** Fully repair all sections */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void FullRepair();

	// ============================================================
	// Subsystem Management
	// ============================================================

	/** Disable a subsystem */
	UFUNCTION(BlueprintCallable, Category = "Damage|Subsystems")
	void DisableSubsystem(ESubsystemType Subsystem, float EfficiencyLoss = 100.0f);

	/** Repair a subsystem */
	UFUNCTION(BlueprintCallable, Category = "Damage|Subsystems")
	void RepairSubsystem(ESubsystemType Subsystem, float EfficiencyGain = 100.0f);

	/** Check if subsystem is operational */
	UFUNCTION(BlueprintPure, Category = "Damage|Subsystems")
	bool IsSubsystemOperational(ESubsystemType Subsystem) const;

	/** Get subsystem efficiency (0-100) */
	UFUNCTION(BlueprintPure, Category = "Damage|Subsystems")
	float GetSubsystemEfficiency(ESubsystemType Subsystem) const;

	// ============================================================
	// Query Functions
	// ============================================================

	/** Get section integrity percentage (0-1) */
	UFUNCTION(BlueprintPure, Category = "Damage|Query")
	float GetSectionIntegrity(EMechSection Section) const;

	/** Get total hull integrity percentage (0-1) */
	UFUNCTION(BlueprintPure, Category = "Damage|Query")
	float GetTotalIntegrityPercent() const;

	/** Check if section is destroyed */
	UFUNCTION(BlueprintPure, Category = "Damage|Query")
	bool IsSectionDestroyed(EMechSection Section) const;

	/** Check if section is critical (<25% integrity) */
	UFUNCTION(BlueprintPure, Category = "Damage|Query")
	bool IsSectionCritical(EMechSection Section) const;

	/** Get section closest to world location */
	UFUNCTION(BlueprintPure, Category = "Damage|Query")
	EMechSection GetSectionAtLocation(FVector WorldLocation) const;

	// ============================================================
	// Hazard Spawning
	// ============================================================

	/** Spawn a fire hazard at interior location */
	UFUNCTION(BlueprintCallable, Category = "Damage|Hazards")
	void SpawnFireHazard(FVector InteriorLocation);

	/** Spawn a coolant leak at interior location */
	UFUNCTION(BlueprintCallable, Category = "Damage|Hazards")
	void SpawnCoolantLeak(FVector InteriorLocation);

	/** Spawn an electrical arc at interior location */
	UFUNCTION(BlueprintCallable, Category = "Damage|Hazards")
	void SpawnElectricalArc(FVector InteriorLocation);

	// ============================================================
	// Events
	// ============================================================

	/** Called when a section takes damage */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSectionDamaged, EMechSection, Section, float, DamageAmount, float, NewIntegrityPercent);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnSectionDamaged OnSectionDamaged;

	/** Called when a section is destroyed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSectionDestroyed, EMechSection, Section);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnSectionDestroyed OnSectionDestroyed;

	/** Called when a section becomes critical */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSectionCritical, EMechSection, Section);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnSectionCritical OnSectionCritical;

	/** Called when a subsystem fails */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubsystemFailed, ESubsystemType, Subsystem, float, RemainingEfficiency);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnSubsystemFailed OnSubsystemFailed;

	/** Called when a subsystem is repaired */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSubsystemRepaired, ESubsystemType, Subsystem, float, NewEfficiency);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnSubsystemRepaired OnSubsystemRepaired;

	/** Called when a hazard is spawned */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHazardSpawned, FString, HazardType, FVector, Location);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnHazardSpawned OnHazardSpawned;

	/** Called when mech is critically damaged (total integrity <25%) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMechCritical, float, TotalIntegrityPercent);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnMechCritical OnMechCritical;

	/** Called when mech is destroyed (total integrity = 0) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMechDestroyed);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnMechDestroyed OnMechDestroyed;

private:
	// ============================================================
	// Internal Functions
	// ============================================================

	/** Initialize all sections with default values */
	void InitializeSections();

	/** Initialize all subsystems with default values */
	void InitializeSubsystems();

	/** Handle section destruction logic */
	void HandleSectionDestruction(EMechSection Section);

	/** Handle subsystem failure cascade */
	void HandleSubsystemFailure(ESubsystemType Subsystem, EMechSection DamagedSection);

	/** Determine if hazard should spawn based on damage */
	bool ShouldSpawnHazard(float DamageAmount, EMechSection Section) const;

	/** Get random interior spawn point near damaged section */
	FVector GetInteriorSpawnPointForSection(EMechSection Section) const;

	/** Select random hazard type based on damage type */
	void SpawnRandomHazard(FVector Location, EDamageType DamageType);

	// ============================================================
	// State Tracking
	// ============================================================

	/** Track if mech was previously at critical health */
	bool bWasCritical = false;

	/** Active hazard actors */
	UPROPERTY()
	TArray<AActor*> ActiveHazards;
};
