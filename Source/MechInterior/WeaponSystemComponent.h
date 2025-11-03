// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponSystemComponent.generated.h"

/**
 * Weapon types for the mech
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Ballistic UMETA(DisplayName = "Ballistic (Autocannon)"),
	Energy UMETA(DisplayName = "Energy (Laser)"),
	Missile UMETA(DisplayName = "Missile Launcher"),
	Melee UMETA(DisplayName = "Melee (Pile Bunker)")
};

/**
 * Weapon mount locations on mech
 */
UENUM(BlueprintType)
enum class EWeaponMount : uint8
{
	RightArm UMETA(DisplayName = "Right Arm"),
	LeftArm UMETA(DisplayName = "Left Arm"),
	RightShoulder UMETA(DisplayName = "Right Shoulder"),
	LeftShoulder UMETA(DisplayName = "Left Shoulder"),
	Torso UMETA(DisplayName = "Torso Center")
};

/**
 * Individual weapon data
 */
USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WeaponName = "Autocannon";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType = EWeaponType::Ballistic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponMount MountLocation = EWeaponMount::RightArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.2f; // Seconds between shots

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 100000.0f; // 1000 meters in cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatPerShot = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo = 200;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentAmmo = 200;

	UPROPERTY(BlueprintReadOnly)
	float LastFireTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsReloading = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 3.0f;
};

/**
 * Weapon System Component
 * Manages all weapons on the mech
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MECHINTERIOR_API UWeaponSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponSystemComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================
	// Weapon Configuration
	// ============================================================

	/** All weapons equipped on this mech */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<FWeaponData> EquippedWeapons;

	/** Currently selected weapon index */
	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	int32 CurrentWeaponIndex = 0;

	// ============================================================
	// Weapon Actions
	// ============================================================

	/** Fire current weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void FireWeapon();

	/** Fire specific weapon by index */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void FireWeaponByIndex(int32 WeaponIndex);

	/** Fire all weapons (alpha strike) */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void FireAllWeapons();

	/** Stop firing current weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void StopFiring();

	/** Reload current weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void ReloadWeapon();

	/** Reload specific weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void ReloadWeaponByIndex(int32 WeaponIndex);

	/** Switch to next weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void NextWeapon();

	/** Switch to previous weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void PreviousWeapon();

	/** Select weapon by index */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void SelectWeapon(int32 WeaponIndex);

	// ============================================================
	// Weapon State Queries
	// ============================================================

	/** Get current weapon data */
	UFUNCTION(BlueprintPure, Category = "Weapons")
	FWeaponData GetCurrentWeapon() const;

	/** Check if current weapon can fire */
	UFUNCTION(BlueprintPure, Category = "Weapons")
	bool CanFireCurrentWeapon() const;

	/** Check if specific weapon can fire */
	UFUNCTION(BlueprintPure, Category = "Weapons")
	bool CanFireWeapon(int32 WeaponIndex) const;

	/** Get total ammo percentage across all weapons */
	UFUNCTION(BlueprintPure, Category = "Weapons")
	float GetTotalAmmoPercentage() const;

	// ============================================================
	// Events
	// ============================================================

	/** Called when a weapon fires */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFired, int32, WeaponIndex, FWeaponData, WeaponData);
	UPROPERTY(BlueprintAssignable, Category = "Weapons")
	FOnWeaponFired OnWeaponFired;

	/** Called when a weapon hits a target */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWeaponHit, int32, WeaponIndex, AActor*, HitActor, float, DamageDealt);
	UPROPERTY(BlueprintAssignable, Category = "Weapons")
	FOnWeaponHit OnWeaponHit;

	/** Called when weapon runs out of ammo */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEmpty, int32, WeaponIndex);
	UPROPERTY(BlueprintAssignable, Category = "Weapons")
	FOnWeaponEmpty OnWeaponEmpty;

	/** Called when weapon reload starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponReloadStart, int32, WeaponIndex, float, ReloadTime);
	UPROPERTY(BlueprintAssignable, Category = "Weapons")
	FOnWeaponReloadStart OnWeaponReloadStart;

	/** Called when weapon reload completes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadComplete, int32, WeaponIndex);
	UPROPERTY(BlueprintAssignable, Category = "Weapons")
	FOnWeaponReloadComplete OnWeaponReloadComplete;

	/** Called when weapon selection changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChanged, int32, OldIndex, int32, NewIndex);
	UPROPERTY(BlueprintAssignable, Category = "Weapons")
	FOnWeaponChanged OnWeaponChanged;

private:
	// ============================================================
	// Internal State
	// ============================================================

	/** Is trigger held down for continuous fire */
	bool bIsFiring = false;

	/** Time accumulator for fire rate */
	float FireAccumulator = 0.0f;

	/** Reload timers per weapon */
	TArray<float> ReloadTimers;

	// ============================================================
	// Internal Functions
	// ============================================================

	/** Perform actual weapon firing logic */
	void ExecuteWeaponFire(int32 WeaponIndex);

	/** Perform raycast for weapon hit detection */
	bool PerformWeaponTrace(const FWeaponData& Weapon, FHitResult& OutHit);

	/** Get weapon muzzle location */
	FVector GetWeaponMuzzleLocation(EWeaponMount Mount) const;

	/** Get weapon aim direction */
	FVector GetWeaponAimDirection() const;

	/** Apply weapon effects (VFX, SFX, damage) */
	void ApplyWeaponEffects(int32 WeaponIndex, const FHitResult& Hit);

	/** Update reload timers */
	void UpdateReloadTimers(float DeltaTime);

	/** Initialize default loadout if none configured */
	void InitializeDefaultLoadout();
};
