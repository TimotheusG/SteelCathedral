// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoolantLeakActor.generated.h"

class UParticleSystemComponent;
class UAudioComponent;
class UDecalComponent;
class USphereComponent;
class UReactorSystemComponent;

/**
 * Coolant leak severity
 */
UENUM(BlueprintType)
enum class ECoolantLeakSeverity : uint8
{
	Minor UMETA(DisplayName = "Minor Leak"),
	Moderate UMETA(DisplayName = "Moderate Leak"),
	Major UMETA(DisplayName = "Major Leak"),
	Critical UMETA(DisplayName = "Critical Leak")
};

/**
 * Interior coolant leak hazard
 * Sprays coolant vapor, increases reactor heat, creates puddles
 * Can be sealed by Technician with repair tool
 */
UCLASS()
class MECHINTERIOR_API ACoolantLeakActor : public AActor
{
	GENERATED_BODY()

public:
	ACoolantLeakActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================
	// Configuration
	// ============================================================

	/** Initial leak severity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	ECoolantLeakSeverity InitialSeverity = ECoolantLeakSeverity::Minor;

	/** Heat increase per second (added to reactor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	float HeatIncreasePerSecond = 2.0f;

	/** Radius of coolant vapor effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	float VaporRadius = 150.0f;

	/** Time to fully seal leak (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	float SealDuration = 5.0f;

	/** Can leak worsen over time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	bool bCanWorsen = true;

	/** Time before leak worsens (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	float TimeBeforeWorsening = 30.0f;

	/** Puddle size (decal scale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	float PuddleSize = 200.0f;

	/** Damage per second to characters standing in vapor (cold damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant|Config")
	float VaporDamagePerSecond = 2.0f;

	// ============================================================
	// State
	// ============================================================

	/** Current leak severity */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Coolant|State")
	ECoolantLeakSeverity CurrentSeverity;

	/** Is leak currently being sealed */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Coolant|State")
	bool bIsBeingSealed = false;

	/** Seal progress (0-1) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Coolant|State")
	float SealProgress = 0.0f;

	/** Time this leak has been active */
	UPROPERTY(BlueprintReadOnly, Category = "Coolant|State")
	float LeakTime = 0.0f;

	/** Has this leak worsened yet */
	UPROPERTY(BlueprintReadOnly, Category = "Coolant|State")
	bool bHasWorsened = false;

	// ============================================================
	// Interaction
	// ============================================================

	/** Start sealing the leak (called by Technician tool) */
	UFUNCTION(BlueprintCallable, Category = "Coolant")
	void StartSealing();

	/** Stop sealing the leak */
	UFUNCTION(BlueprintCallable, Category = "Coolant")
	void StopSealing();

	/** Apply sealant (reduces severity) */
	UFUNCTION(BlueprintCallable, Category = "Coolant")
	void ApplySealant(float DeltaTime);

	/** Instantly seal the leak (debug/cheat) */
	UFUNCTION(BlueprintCallable, Category = "Coolant")
	void SealImmediately();

	/** Worsen the leak severity */
	UFUNCTION(BlueprintCallable, Category = "Coolant")
	void WorsenLeak();

	/** Improve the leak severity */
	UFUNCTION(BlueprintCallable, Category = "Coolant")
	void ImproveLeak();

	// ============================================================
	// Query
	// ============================================================

	/** Get current heat increase per second based on severity */
	UFUNCTION(BlueprintPure, Category = "Coolant")
	float GetCurrentHeatIncrease() const;

	/** Get current vapor damage based on severity */
	UFUNCTION(BlueprintPure, Category = "Coolant")
	float GetCurrentVaporDamage() const;

	/** Check if leak can be interacted with */
	UFUNCTION(BlueprintPure, Category = "Coolant")
	bool CanBeSealed() const;

	/** Get severity as multiplier (1-4) */
	UFUNCTION(BlueprintPure, Category = "Coolant")
	float GetSeverityMultiplier() const;

	// ============================================================
	// Events
	// ============================================================

	/** Called when leak starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeakStarted, ACoolantLeakActor*, Leak);
	UPROPERTY(BlueprintAssignable, Category = "Coolant|Events")
	FOnLeakStarted OnLeakStarted;

	/** Called when leak is sealed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeakSealed, ACoolantLeakActor*, Leak);
	UPROPERTY(BlueprintAssignable, Category = "Coolant|Events")
	FOnLeakSealed OnLeakSealed;

	/** Called when leak severity changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeverityChanged, ECoolantLeakSeverity, OldSeverity, ECoolantLeakSeverity, NewSeverity);
	UPROPERTY(BlueprintAssignable, Category = "Coolant|Events")
	FOnSeverityChanged OnSeverityChanged;

	/** Called when character enters vapor cloud */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterInVapor, AActor*, Character);
	UPROPERTY(BlueprintAssignable, Category = "Coolant|Events")
	FOnCharacterInVapor OnCharacterInVapor;

private:
	// ============================================================
	// Components
	// ============================================================

	/** Root component */
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	/** Coolant vapor spray particle effect */
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* VaporParticles;

	/** Coolant hissing audio */
	UPROPERTY(VisibleAnywhere)
	UAudioComponent* LeakAudio;

	/** Floor puddle decal */
	UPROPERTY(VisibleAnywhere)
	UDecalComponent* PuddleDecal;

	/** Vapor damage detection sphere */
	UPROPERTY(VisibleAnywhere)
	USphereComponent* VaporSphere;

	// ============================================================
	// Internal Functions
	// ============================================================

	/** Update leak visuals based on severity */
	void UpdateLeakVisuals();

	/** Update leak audio based on severity */
	void UpdateLeakAudio();

	/** Update puddle size based on severity */
	void UpdatePuddle();

	/** Apply heat to reactor system */
	void ApplyHeatToReactor(float DeltaTime);

	/** Apply damage to actors in vapor cloud */
	void ApplyVaporDamage(float DeltaTime);

	/** Check if leak should worsen */
	void CheckForWorsening(float DeltaTime);

	/** Handle leak being fully sealed */
	void HandleSealed();

	// ============================================================
	// Overlap Events
	// ============================================================

	UFUNCTION()
	void OnVaporSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                                bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVaporSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ============================================================
	// State Tracking
	// ============================================================

	/** Actors currently in vapor cloud */
	UPROPERTY()
	TArray<AActor*> ActorsInVapor;

	/** Accumulated damage time (for tick-rate independent damage) */
	float AccumulatedDamageTime = 0.0f;

	/** Cached reactor system reference */
	UPROPERTY()
	UReactorSystemComponent* CachedReactor = nullptr;
};
