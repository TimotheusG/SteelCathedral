// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UMechMovementComponent::UMechMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Enable replication
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UMechMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("MechMovementComponent initialized"));
}

// Called every frame
void UMechMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only process movement on authority (server or standalone)
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Update balance
	UpdateBalance(DeltaTime);

	// Update boost cooldown
	if (BoostCooldownRemaining > 0)
	{
		BoostCooldownRemaining -= DeltaTime;
	}

	// Handle boost
	if (bIsBoostActive)
	{
		BoostTimeRemaining -= DeltaTime;
		if (BoostTimeRemaining <= 0)
		{
			bIsBoostActive = false;
			MovementState = EMoveState::Idle;
		}
	}

	// Update movement based on input
	EMoveState TargetState = EMoveState::Idle;
	float TargetSpeed = 0.0f;

	if (bIsBraced)
	{
		TargetState = EMoveState::Braced;
		TargetSpeed = 0.0f;
	}
	else if (bIsBoostActive)
	{
		TargetState = EMoveState::Boosting;
		TargetSpeed = BoostSpeed;
	}
	else if (FMath::Abs(MoveForwardInput) > 0.1f)
	{
		TargetState = EMoveState::Walking;
		TargetSpeed = MaxWalkSpeed * FMath::Abs(MoveForwardInput);
	}

	// Apply terrain modifier
	float TerrainModifier = GetTerrainSpeedModifier();
	TargetSpeed *= TerrainModifier;

	// Interpolate to target speed
	float AccelerationRate = (TargetSpeed > CurrentSpeed) ? 2.0f : 3.0f;
	CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, AccelerationRate);

	// Update movement state
	if (MovementState != TargetState)
	{
		PreviousMovementState = MovementState;
		MovementState = TargetState;
		OnMovementStateChanged.Broadcast(MovementState);
	}

	// Apply movement to owner actor
	AActor* OwnerActor = GetOwner();
	if (OwnerActor && CurrentSpeed > 0.1f)
	{
		FVector ForwardVector = OwnerActor->GetActorForwardVector();

		// Use boost direction if boosting, otherwise use forward input
		if (bIsBoostActive)
		{
			ForwardVector = BoostDirection;
		}
		else
		{
			ForwardVector *= FMath::Sign(MoveForwardInput);
		}

		FVector MovementDelta = ForwardVector * CurrentSpeed * DeltaTime;
		OwnerActor->AddActorWorldOffset(MovementDelta, true);

		// Footstep handling
		FootstepTimer += DeltaTime;
		float CurrentFootstepInterval = FootstepInterval * (MaxWalkSpeed / FMath::Max(CurrentSpeed, 1.0f));
		if (FootstepTimer >= CurrentFootstepInterval)
		{
			FootstepTimer = 0.0f;
			OnFootstep.Broadcast();
		}
	}

	// Apply rotation
	if (FMath::Abs(TurnInputValue) > 0.01f)
	{
		float RotationDelta = TurnInputValue * TurnRate * DeltaTime;
		FRotator NewRotation = OwnerActor->GetActorRotation();
		NewRotation.Yaw += RotationDelta;
		OwnerActor->SetActorRotation(NewRotation);
	}
}

void UMechMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMechMovementComponent, CurrentSpeed);
	DOREPLIFETIME(UMechMovementComponent, BalanceValue);
	DOREPLIFETIME(UMechMovementComponent, MovementState);
}

void UMechMovementComponent::ProcessMoveInput(FVector2D InputVector)
{
	MoveForwardInput = InputVector.X;
	// InputVector.Y could be used for strafing in future implementation
}

void UMechMovementComponent::ProcessTurnInput(float TurnInput)
{
	TurnInputValue = TurnInput;
}

void UMechMovementComponent::SetBraced(bool bBraced)
{
	bIsBraced = bBraced;

	if (bBraced)
	{
		MovementState = EMoveState::Braced;
		OnMovementStateChanged.Broadcast(MovementState);
	}
}

void UMechMovementComponent::ActivateBoost(FVector Direction)
{
	if (!CanBoost())
	{
		return;
	}

	bIsBoostActive = true;
	BoostTimeRemaining = BoostDuration;
	BoostCooldownRemaining = BoostCooldown;
	BoostDirection = Direction.GetSafeNormal();
	MovementState = EMoveState::Boosting;

	OnBoostActivated.Broadcast(BoostHeatGeneration);
	OnMovementStateChanged.Broadcast(MovementState);

	UE_LOG(LogTemp, Warning, TEXT("Boost activated! Heat: %f"), BoostHeatGeneration);
}

bool UMechMovementComponent::IsBraced() const
{
	return bIsBraced;
}

bool UMechMovementComponent::CanBoost() const
{
	return !bIsBoostActive && BoostCooldownRemaining <= 0.0f && !bIsBraced;
}

float UMechMovementComponent::GetAimStabilityModifier() const
{
	switch (MovementState)
	{
	case EMoveState::Idle:
	case EMoveState::Braced:
		return 1.0f; // Perfect stability

	case EMoveState::Walking:
		return 0.6f; // Moderate stability

	case EMoveState::Running:
		return 0.3f; // Low stability

	case EMoveState::Boosting:
	case EMoveState::Stumbling:
		return 0.1f; // Very poor stability

	case EMoveState::Fallen:
		return 0.0f; // No stability

	default:
		return 0.5f;
	}
}

void UMechMovementComponent::UpdateTerrainType(ETerrainType NewTerrain)
{
	if (CurrentTerrain != NewTerrain)
	{
		CurrentTerrain = NewTerrain;
		UE_LOG(LogTemp, Log, TEXT("Terrain changed to: %d"), static_cast<int32>(CurrentTerrain));
	}
}

float UMechMovementComponent::GetTerrainSpeedModifier() const
{
	switch (CurrentTerrain)
	{
	case ETerrainType::Stable:
		return 1.0f;

	case ETerrainType::Loose:
		return 0.7f; // Rubble, debris

	case ETerrainType::Ice:
		return 0.5f; // Very slippery

	case ETerrainType::Lava:
		return 0.4f; // Dangerous, slow movement

	case ETerrainType::Water:
		return 0.6f; // Water resistance

	case ETerrainType::Hazardous:
		return 0.5f; // General hazards

	default:
		return 1.0f;
	}
}

void UMechMovementComponent::ApplyBalanceDisruption(float Magnitude, FVector Direction)
{
	// Calculate disruption based on direction (forward = positive, backward = negative)
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		FVector ForwardVector = OwnerActor->GetActorForwardVector();
		float DotProduct = FVector::DotProduct(Direction.GetSafeNormal(), ForwardVector);

		BalanceValue += DotProduct * Magnitude;
		BalanceValue = FMath::Clamp(BalanceValue, -1.0f, 1.0f);

		// Check for stumble
		if (BalanceValue <= StumbleThreshold && MovementState != EMoveState::Stumbling)
		{
			MovementState = EMoveState::Stumbling;
			OnBalanceLost.Broadcast();
			UE_LOG(LogTemp, Warning, TEXT("Mech stumbling! Balance: %f"), BalanceValue);
		}
	}
}

void UMechMovementComponent::UpdateBalance(float DeltaTime)
{
	// Naturally recover balance toward center
	if (FMath::Abs(BalanceValue) > 0.01f)
	{
		float RecoveryDirection = -FMath::Sign(BalanceValue);
		BalanceValue += RecoveryDirection * BalanceRecoveryRate * DeltaTime;

		// Clamp to zero if very close
		if (FMath::Abs(BalanceValue) < 0.01f)
		{
			BalanceValue = 0.0f;
		}
	}

	// Check if recovered from stumble
	if (MovementState == EMoveState::Stumbling && BalanceValue > StumbleThreshold)
	{
		MovementState = EMoveState::Idle;
		OnMovementStateChanged.Broadcast(MovementState);
	}
}
