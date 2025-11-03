// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReactorSystemComponent.generated.h"

/** Reactor states based on heat level */
UENUM(BlueprintType)
enum class EReactorState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Warning UMETA(DisplayName = "Warning"),
	Danger UMETA(DisplayName = "Danger"),
	Critical UMETA(DisplayName = "Critical"),
	Meltdown UMETA(DisplayName = "Meltdown")
};

/** System types for power allocation */
UENUM(BlueprintType)
enum class ESystemType : uint8
{
	Movement UMETA(DisplayName = "Movement"),
	Weapons UMETA(DisplayName = "Weapons"),
	Reactor UMETA(DisplayName = "Reactor"),
	LifeSupport UMETA(DisplayName = "LifeSupport"),
	Sensors UMETA(DisplayName = "Sensors"),
	Communications UMETA(DisplayName = "Communications"),
	Hydraulics UMETA(DisplayName = "Hydraulics"),
	Auxiliary UMETA(DisplayName = "Auxiliary")
};

/**
 * Reactor system component for power generation and heat management
 * Core of the mech's energy systems - generates heat and distributes power
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MECHINTERIOR_API UReactorSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UReactorSystemComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Setup replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// ============================================================
	// Reactor Parameters
	// ============================================================

	/** Maximum reactor heat (100%) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Configuration")
	float MaxReactorHeat = 100.0f;

	/** Maximum reactor power (100%) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Configuration")
	float MaxReactorPower = 100.0f;

	/** Base heat generation per second (idle) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Configuration")
	float BaseHeatGeneration = 1.0f;

	/** Passive cooling rate per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Configuration")
	float PassiveCoolingRate = 0.5f;

	/** Heat removed by manual venting */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Configuration")
	float VentCoolingAmount = 30.0f;

	/** Duration of venting process in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Configuration")
	float VentDuration = 10.0f;

	/** Power reduction during venting (percentage) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Configuration")
	float VentPowerReduction = 25.0f;

	// ============================================================
	// Heat Thresholds
	// ============================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Thresholds")
	float WarningThreshold = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Thresholds")
	float DangerThreshold = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Thresholds")
	float CriticalThreshold = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactor|Thresholds")
	float MeltdownThreshold = 100.0f;

	// ============================================================
	// Current State
	// ============================================================

	/** Current reactor heat (0-100) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Reactor|State")
	float ReactorHeat = 0.0f;

	/** Current reactor power (0-100) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Reactor|State")
	float ReactorPower = 100.0f;

	/** Is venting currently in progress */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Reactor|State")
	bool bVentingInProgress = false;

	/** Remaining time for current venting process */
	UPROPERTY(BlueprintReadOnly, Category = "Reactor|State")
	float VentingTimeRemaining = 0.0f;

	// ============================================================
	// Heat Management
	// ============================================================

	/** Add heat to the reactor */
	UFUNCTION(BlueprintCallable, Category = "Reactor")
	void AddHeat(float HeatAmount);

	/** Reduce heat from the reactor */
	UFUNCTION(BlueprintCallable, Category = "Reactor")
	void ReduceHeat(float HeatAmount);

	/** Initiate manual venting */
	UFUNCTION(BlueprintCallable, Category = "Reactor")
	void InitiateVenting();

	/** Server RPC for venting */
	UFUNCTION(Server, Reliable)
	void Server_InitiateVenting();

	/** Complete venting process */
	void CompleteVenting();

	/** Apply environmental heat (e.g., from lava terrain) */
	UFUNCTION(BlueprintCallable, Category = "Reactor")
	void ApplyEnvironmentalHeat(float HeatPerSecond);

	// ============================================================
	// Power Management
	// ============================================================

	/** Reallocate power to a specific system */
	UFUNCTION(BlueprintCallable, Category = "Reactor")
	void ReallocatePower(ESystemType SystemType, float PowerPercentage);

	/** Divert power from a system (emergency) */
	UFUNCTION(BlueprintCallable, Category = "Reactor")
	void DivertPowerFromSystem(ESystemType SystemType);

	/** Get available power for a specific system */
	UFUNCTION(BlueprintPure, Category = "Reactor")
	float GetAvailablePowerForSystem(ESystemType SystemType) const;

	// ============================================================
	// State Queries
	// ============================================================

	/** Get heat as percentage (0-100) */
	UFUNCTION(BlueprintPure, Category = "Reactor")
	float GetHeatPercentage() const;

	/** Get power as percentage (0-100) */
	UFUNCTION(BlueprintPure, Category = "Reactor")
	float GetPowerPercentage() const;

	/** Get current reactor state based on heat */
	UFUNCTION(BlueprintPure, Category = "Reactor")
	EReactorState GetReactorState() const;

	/** Check if venting is possible */
	UFUNCTION(BlueprintPure, Category = "Reactor")
	bool CanVent() const;

	// ============================================================
	// Events
	// ============================================================

	/** Broadcast when heat threshold is reached */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeatThresholdReached, EReactorState, NewState);
	UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
	FOnHeatThresholdReached OnHeatThresholdReached;

	/** Broadcast when venting starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVentingStarted);
	UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
	FOnVentingStarted OnVentingStarted;

	/** Broadcast when venting completes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVentingCompleted);
	UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
	FOnVentingCompleted OnVentingCompleted;

	/** Broadcast when meltdown is initiated */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeltdownInitiated);
	UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
	FOnMeltdownInitiated OnMeltdownInitiated;

	/** Broadcast when power is reduced to a system */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerReduced, ESystemType, AffectedSystem, float, NewPowerLevel);
	UPROPERTY(BlueprintAssignable, Category = "Reactor|Events")
	FOnPowerReduced OnPowerReduced;

private:
	// Internal state tracking
	EReactorState PreviousReactorState = EReactorState::Normal;
	TMap<ESystemType, float> PowerAllocationMap;
	float EnvironmentalHeatRate = 0.0f;
};
