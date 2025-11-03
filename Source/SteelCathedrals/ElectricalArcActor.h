// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElectricalArcActor.generated.h"

class UParticleSystemComponent;
class UAudioComponent;
class USphereComponent;
class UPointLightComponent;

/**
 * Electrical arc intensity
 */
UENUM(BlueprintType)
enum class EArcIntensity : uint8
{
	Weak UMETA(DisplayName = "Weak Arc"),
	Moderate UMETA(DisplayName = "Moderate Arc"),
	Strong UMETA(DisplayName = "Strong Arc"),
	Lethal UMETA(DisplayName = "Lethal Arc")
};

/**
 * Interior electrical hazard
 * Damages characters, disables nearby systems, sparks and arcs
 * Can be repaired by Technician with electrical tools
 */
UCLASS()
class STEELCATHEDRALS_API AElectricalArcActor : public AActor
{
	GENERATED_BODY()

public:
	AElectricalArcActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================
	// Configuration
	// ============================================================

	/** Initial arc intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	EArcIntensity InitialIntensity = EArcIntensity::Weak;

	/** Damage per second to characters in range (electrical shock) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	float DamagePerSecond = 8.0f;

	/** Radius of electrical damage effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	float DamageRadius = 150.0f;

	/** Time between arc pulses (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	float ArcPulseInterval = 0.5f;

	/** Chance to disable nearby systems per pulse (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	float SystemDisruptionChance = 0.2f;

	/** Duration of system disruption (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	float DisruptionDuration = 3.0f;

	/** Time to fully repair arc (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	float RepairDuration = 4.0f;

	/** Can arc cause fires nearby */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	bool bCanIgniteFires = true;

	/** Chance to start fire per pulse (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Config")
	float FireIgnitionChance = 0.05f;

	// ============================================================
	// State
	// ============================================================

	/** Current arc intensity */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Electrical|State")
	EArcIntensity CurrentIntensity;

	/** Is arc currently being repaired */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Electrical|State")
	bool bIsBeingRepaired = false;

	/** Repair progress (0-1) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Electrical|State")
	float RepairProgress = 0.0f;

	/** Time since last arc pulse */
	UPROPERTY(BlueprintReadOnly, Category = "Electrical|State")
	float TimeSinceLastPulse = 0.0f;

	/** Is arc currently pulsing (visual effect) */
	UPROPERTY(BlueprintReadOnly, Category = "Electrical|State")
	bool bIsPulsing = false;

	/** Time this arc has been active */
	UPROPERTY(BlueprintReadOnly, Category = "Electrical|State")
	float ArcTime = 0.0f;

	// ============================================================
	// Interaction
	// ============================================================

	/** Start repairing the arc (called by Technician tool) */
	UFUNCTION(BlueprintCallable, Category = "Electrical")
	void StartRepairing();

	/** Stop repairing the arc */
	UFUNCTION(BlueprintCallable, Category = "Electrical")
	void StopRepairing();

	/** Apply electrical repair (reduces intensity) */
	UFUNCTION(BlueprintCallable, Category = "Electrical")
	void ApplyRepair(float DeltaTime);

	/** Instantly repair the arc (debug/cheat) */
	UFUNCTION(BlueprintCallable, Category = "Electrical")
	void RepairImmediately();

	/** Increase arc intensity */
	UFUNCTION(BlueprintCallable, Category = "Electrical")
	void IncreaseIntensity();

	/** Decrease arc intensity */
	UFUNCTION(BlueprintCallable, Category = "Electrical")
	void DecreaseIntensity();

	// ============================================================
	// Query
	// ============================================================

	/** Get current damage per second based on intensity */
	UFUNCTION(BlueprintPure, Category = "Electrical")
	float GetCurrentDamagePerSecond() const;

	/** Get current system disruption chance based on intensity */
	UFUNCTION(BlueprintPure, Category = "Electrical")
	float GetCurrentDisruptionChance() const;

	/** Check if arc can be interacted with */
	UFUNCTION(BlueprintPure, Category = "Electrical")
	bool CanBeRepaired() const;

	/** Get intensity as multiplier (1-4) */
	UFUNCTION(BlueprintPure, Category = "Electrical")
	float GetIntensityMultiplier() const;

	// ============================================================
	// Events
	// ============================================================

	/** Called when arc starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArcStarted, AElectricalArcActor*, Arc);
	UPROPERTY(BlueprintAssignable, Category = "Electrical|Events")
	FOnArcStarted OnArcStarted;

	/** Called when arc is repaired */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArcRepaired, AElectricalArcActor*, Arc);
	UPROPERTY(BlueprintAssignable, Category = "Electrical|Events")
	FOnArcRepaired OnArcRepaired;

	/** Called when arc intensity changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIntensityChanged, EArcIntensity, OldIntensity, EArcIntensity, NewIntensity);
	UPROPERTY(BlueprintAssignable, Category = "Electrical|Events")
	FOnIntensityChanged OnIntensityChanged;

	/** Called on each arc pulse */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnArcPulse);
	UPROPERTY(BlueprintAssignable, Category = "Electrical|Events")
	FOnArcPulse OnArcPulse;

	/** Called when character is shocked */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterShocked, AActor*, Character);
	UPROPERTY(BlueprintAssignable, Category = "Electrical|Events")
	FOnCharacterShocked OnCharacterShocked;

	/** Called when nearby system is disrupted */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemDisrupted, FString, SystemName);
	UPROPERTY(BlueprintAssignable, Category = "Electrical|Events")
	FOnSystemDisrupted OnSystemDisrupted;

private:
	// ============================================================
	// Components
	// ============================================================

	/** Root component */
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	/** Electrical arc particle effect */
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ArcParticles;

	/** Electrical buzzing/crackling audio */
	UPROPERTY(VisibleAnywhere)
	UAudioComponent* ArcAudio;

	/** Flickering light from arc */
	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* ArcLight;

	/** Damage detection sphere */
	UPROPERTY(VisibleAnywhere)
	USphereComponent* DamageSphere;

	// ============================================================
	// Internal Functions
	// ============================================================

	/** Update arc visuals based on intensity */
	void UpdateArcVisuals();

	/** Update arc audio based on intensity */
	void UpdateArcAudio();

	/** Update arc light flickering */
	void UpdateArcLight(float DeltaTime);

	/** Trigger arc pulse */
	void TriggerArcPulse();

	/** Apply damage to nearby actors */
	void ApplyElectricalDamage();

	/** Attempt to disrupt nearby systems */
	void AttemptSystemDisruption();

	/** Attempt to ignite nearby fires */
	void AttemptFireIgnition();

	/** Handle arc being fully repaired */
	void HandleRepaired();

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

	/** Actors currently in arc damage range */
	UPROPERTY()
	TArray<AActor*> ActorsInRange;

	/** Light flicker timer */
	float LightFlickerTimer = 0.0f;

	/** Random light flicker interval */
	float LightFlickerInterval = 0.1f;
};
