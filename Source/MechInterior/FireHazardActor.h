// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireHazardActor.generated.h"

class UParticleSystemComponent;
class UAudioComponent;
class USphereComponent;

/**
 * Fire intensity levels
 */
UENUM(BlueprintType)
enum class EFireIntensity : uint8
{
	Small UMETA(DisplayName = "Small Fire"),
	Medium UMETA(DisplayName = "Medium Fire"),
	Large UMETA(DisplayName = "Large Fire"),
	Inferno UMETA(DisplayName = "Inferno")
};

/**
 * Interior fire hazard that damages systems and spreads
 * Can be extinguished by Technician with fire extinguisher
 */
UCLASS()
class MECHINTERIOR_API AFireHazardActor : public AActor
{
	GENERATED_BODY()

public:
	AFireHazardActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================
	// Configuration
	// ============================================================

	/** Initial fire intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	EFireIntensity InitialIntensity = EFireIntensity::Small;

	/** Damage per second to characters in range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	float DamagePerSecond = 5.0f;

	/** Heat generation per second (affects reactor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	float HeatGenerationPerSecond = 2.0f;

	/** Radius of damage effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	float DamageRadius = 200.0f;

	/** Time before fire starts spreading (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	float TimeBeforeSpreading = 10.0f;

	/** Chance to spread per second (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	float SpreadChancePerSecond = 0.1f;

	/** Maximum distance fire can spread */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	float MaxSpreadDistance = 300.0f;

	/** Time to fully extinguish fire (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	float ExtinguishDuration = 3.0f;

	/** Can this fire spread along cables/walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire|Config")
	bool bCanSpread = true;

	// ============================================================
	// State
	// ============================================================

	/** Current fire intensity */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fire|State")
	EFireIntensity CurrentIntensity;

	/** Is fire currently being extinguished */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fire|State")
	bool bIsBeingExtinguished = false;

	/** Extinguish progress (0-1) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Fire|State")
	float ExtinguishProgress = 0.0f;

	/** Time this fire has been burning */
	UPROPERTY(BlueprintReadOnly, Category = "Fire|State")
	float BurnTime = 0.0f;

	/** Has this fire spawned a spread yet */
	UPROPERTY(BlueprintReadOnly, Category = "Fire|State")
	bool bHasSpread = false;

	// ============================================================
	// Interaction
	// ============================================================

	/** Start extinguishing the fire (called by Technician tool) */
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void StartExtinguishing();

	/** Stop extinguishing the fire */
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void StopExtinguishing();

	/** Apply extinguishing agent (reduces intensity) */
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void ApplyExtinguisher(float DeltaTime);

	/** Instantly put out the fire (debug/cheat) */
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void ExtinguishImmediately();

	/** Increase fire intensity */
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void IncreaseIntensity();

	/** Decrease fire intensity */
	UFUNCTION(BlueprintCallable, Category = "Fire")
	void DecreaseIntensity();

	// ============================================================
	// Query
	// ============================================================

	/** Get current damage per second based on intensity */
	UFUNCTION(BlueprintPure, Category = "Fire")
	float GetCurrentDamagePerSecond() const;

	/** Get current heat generation based on intensity */
	UFUNCTION(BlueprintPure, Category = "Fire")
	float GetCurrentHeatGeneration() const;

	/** Check if fire can be interacted with */
	UFUNCTION(BlueprintPure, Category = "Fire")
	bool CanBeExtinguished() const;

	/** Get intensity as multiplier (1-4) */
	UFUNCTION(BlueprintPure, Category = "Fire")
	float GetIntensityMultiplier() const;

	// ============================================================
	// Events
	// ============================================================

	/** Called when fire starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireStarted, AFireHazardActor*, Fire);
	UPROPERTY(BlueprintAssignable, Category = "Fire|Events")
	FOnFireStarted OnFireStarted;

	/** Called when fire is extinguished */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireExtinguished, AFireHazardActor*, Fire);
	UPROPERTY(BlueprintAssignable, Category = "Fire|Events")
	FOnFireExtinguished OnFireExtinguished;

	/** Called when fire intensity changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIntensityChanged, EFireIntensity, OldIntensity, EFireIntensity, NewIntensity);
	UPROPERTY(BlueprintAssignable, Category = "Fire|Events")
	FOnIntensityChanged OnIntensityChanged;

	/** Called when fire spreads */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFireSpread, AFireHazardActor*, OriginalFire, AFireHazardActor*, NewFire);
	UPROPERTY(BlueprintAssignable, Category = "Fire|Events")
	FOnFireSpread OnFireSpread;

	/** Called when character enters fire damage range */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterBurned, AActor*, Character);
	UPROPERTY(BlueprintAssignable, Category = "Fire|Events")
	FOnCharacterBurned OnCharacterBurned;

private:
	// ============================================================
	// Components
	// ============================================================

	/** Root component */
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	/** Fire particle effect */
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* FireParticles;

	/** Fire audio (crackling) */
	UPROPERTY(VisibleAnywhere)
	UAudioComponent* FireAudio;

	/** Damage detection sphere */
	UPROPERTY(VisibleAnywhere)
	USphereComponent* DamageSphere;

	// ============================================================
	// Internal Functions
	// ============================================================

	/** Update fire visuals based on intensity */
	void UpdateFireVisuals();

	/** Update fire audio based on intensity */
	void UpdateFireAudio();

	/** Apply damage to nearby actors */
	void ApplyFireDamage(float DeltaTime);

	/** Check if fire should spread */
	void CheckForSpreading(float DeltaTime);

	/** Spawn a new fire at spread location */
	void SpreadFire(FVector SpreadLocation);

	/** Handle fire reaching zero intensity */
	void HandleExtinguished();

	// ============================================================
	// Overlap Events
	// ============================================================

	UFUNCTION()
	void OnDamageSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                                 bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDamageSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ============================================================
	// State Tracking
	// ============================================================

	/** Actors currently in fire damage range */
	UPROPERTY()
	TArray<AActor*> ActorsInRange;

	/** Accumulated damage time (for tick-rate independent damage) */
	float AccumulatedDamageTime = 0.0f;

	/** Parent fire that spawned this one (for spread tracking) */
	UPROPERTY()
	AFireHazardActor* ParentFire = nullptr;
};
