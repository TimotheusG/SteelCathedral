// Copyright Epic Games, Inc. All Rights Reserved.

#include "ReactorSystemComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UReactorSystemComponent::UReactorSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Enable replication
	SetIsReplicatedByDefault(true);

	// Initialize power allocation (default equal distribution)
	PowerAllocationMap.Add(ESystemType::Movement, 15.0f);
	PowerAllocationMap.Add(ESystemType::Weapons, 30.0f);
	PowerAllocationMap.Add(ESystemType::Reactor, 10.0f);
	PowerAllocationMap.Add(ESystemType::LifeSupport, 10.0f);
	PowerAllocationMap.Add(ESystemType::Sensors, 10.0f);
	PowerAllocationMap.Add(ESystemType::Communications, 5.0f);
	PowerAllocationMap.Add(ESystemType::Hydraulics, 15.0f);
	PowerAllocationMap.Add(ESystemType::Auxiliary, 5.0f);
}

// Called when the game starts
void UReactorSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ReactorSystemComponent initialized - Power: %f, Heat: %f"), ReactorPower, ReactorHeat);
}

// Called every frame
void UReactorSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only update on authority (server or standalone)
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Apply base heat generation
	ReactorHeat += BaseHeatGeneration * DeltaTime;

	// Apply environmental heat
	if (EnvironmentalHeatRate > 0.0f)
	{
		ReactorHeat += EnvironmentalHeatRate * DeltaTime;
	}

	// Apply cooling
	if (bVentingInProgress)
	{
		// Active venting
		VentingTimeRemaining -= DeltaTime;
		float VentingCoolingRate = VentCoolingAmount / VentDuration;
		ReactorHeat = FMath::Max(0.0f, ReactorHeat - (VentingCoolingRate * DeltaTime));

		// Reduce power during venting
		ReactorPower = MaxReactorPower - VentPowerReduction;

		if (VentingTimeRemaining <= 0.0f)
		{
			CompleteVenting();
		}
	}
	else
	{
		// Passive cooling
		ReactorHeat = FMath::Max(0.0f, ReactorHeat - (PassiveCoolingRate * DeltaTime));

		// Restore power if not venting
		if (ReactorPower < MaxReactorPower)
		{
			ReactorPower = FMath::Min(MaxReactorPower, ReactorPower + (10.0f * DeltaTime));
		}
	}

	// Clamp heat
	ReactorHeat = FMath::Clamp(ReactorHeat, 0.0f, MaxReactorHeat);

	// Check for state changes
	EReactorState CurrentState = GetReactorState();
	if (CurrentState != PreviousReactorState)
	{
		OnHeatThresholdReached.Broadcast(CurrentState);
		PreviousReactorState = CurrentState;

		UE_LOG(LogTemp, Warning, TEXT("Reactor state changed to: %d, Heat: %f%%"),
			static_cast<int32>(CurrentState), GetHeatPercentage());
	}

	// Check for meltdown
	if (ReactorHeat >= MeltdownThreshold)
	{
		OnMeltdownInitiated.Broadcast();
		UE_LOG(LogTemp, Error, TEXT("REACTOR MELTDOWN! Heat: %f%%"), GetHeatPercentage());
	}
}

void UReactorSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UReactorSystemComponent, ReactorHeat);
	DOREPLIFETIME(UReactorSystemComponent, ReactorPower);
	DOREPLIFETIME(UReactorSystemComponent, bVentingInProgress);
}

void UReactorSystemComponent::AddHeat(float HeatAmount)
{
	ReactorHeat = FMath::Min(MaxReactorHeat, ReactorHeat + HeatAmount);
	UE_LOG(LogTemp, Log, TEXT("Heat added: +%f, Total: %f%%"), HeatAmount, GetHeatPercentage());
}

void UReactorSystemComponent::ReduceHeat(float HeatAmount)
{
	ReactorHeat = FMath::Max(0.0f, ReactorHeat - HeatAmount);
}

void UReactorSystemComponent::InitiateVenting()
{
	// Call server RPC if this is a client
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		Server_InitiateVenting();
		return;
	}

	if (!CanVent())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot vent - venting already in progress"));
		return;
	}

	bVentingInProgress = true;
	VentingTimeRemaining = VentDuration;
	OnVentingStarted.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("Venting initiated - Duration: %f seconds"), VentDuration);
}

void UReactorSystemComponent::Server_InitiateVenting_Implementation()
{
	InitiateVenting();
}

void UReactorSystemComponent::CompleteVenting()
{
	bVentingInProgress = false;
	VentingTimeRemaining = 0.0f;
	ReactorPower = MaxReactorPower; // Restore power
	OnVentingCompleted.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("Venting completed - Heat: %f%%"), GetHeatPercentage());
}

void UReactorSystemComponent::ApplyEnvironmentalHeat(float HeatPerSecond)
{
	EnvironmentalHeatRate = HeatPerSecond;
	UE_LOG(LogTemp, Log, TEXT("Environmental heat rate set to: %f per second"), HeatPerSecond);
}

void UReactorSystemComponent::ReallocatePower(ESystemType SystemType, float PowerPercentage)
{
	if (PowerAllocationMap.Contains(SystemType))
	{
		PowerAllocationMap[SystemType] = PowerPercentage;
		OnPowerReduced.Broadcast(SystemType, PowerPercentage);
	}
}

void UReactorSystemComponent::DivertPowerFromSystem(ESystemType SystemType)
{
	if (PowerAllocationMap.Contains(SystemType))
	{
		PowerAllocationMap[SystemType] = 0.0f;
		OnPowerReduced.Broadcast(SystemType, 0.0f);
		UE_LOG(LogTemp, Warning, TEXT("Power diverted from system: %d"), static_cast<int32>(SystemType));
	}
}

float UReactorSystemComponent::GetAvailablePowerForSystem(ESystemType SystemType) const
{
	if (PowerAllocationMap.Contains(SystemType))
	{
		// Return allocated power adjusted by current reactor power level
		float AllocatedPercentage = PowerAllocationMap[SystemType];
		return (AllocatedPercentage / 100.0f) * ReactorPower;
	}
	return 0.0f;
}

float UReactorSystemComponent::GetHeatPercentage() const
{
	return (ReactorHeat / MaxReactorHeat) * 100.0f;
}

float UReactorSystemComponent::GetPowerPercentage() const
{
	return (ReactorPower / MaxReactorPower) * 100.0f;
}

EReactorState UReactorSystemComponent::GetReactorState() const
{
	float HeatPercent = GetHeatPercentage();

	if (HeatPercent >= MeltdownThreshold)
	{
		return EReactorState::Meltdown;
	}
	else if (HeatPercent >= CriticalThreshold)
	{
		return EReactorState::Critical;
	}
	else if (HeatPercent >= DangerThreshold)
	{
		return EReactorState::Danger;
	}
	else if (HeatPercent >= WarningThreshold)
	{
		return EReactorState::Warning;
	}
	else
	{
		return EReactorState::Normal;
	}
}

bool UReactorSystemComponent::CanVent() const
{
	return !bVentingInProgress;
}
