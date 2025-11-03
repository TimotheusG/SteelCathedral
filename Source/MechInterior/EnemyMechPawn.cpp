// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyMechPawn.h"
#include "MechMovementComponent.h"
#include "ReactorSystemComponent.h"
#include "WeaponSystemComponent.h"
#include "DamageManagementComponent.h"
#include "ProceduralMechGeometry.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

AEnemyMechPawn::AEnemyMechPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);

	// Create root
	MechRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MechRoot"));
	RootComponent = MechRoot;

	// Create collision capsule (42m tall, 4m radius)
	MechCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MechCapsule"));
	MechCapsule->SetupAttachment(RootComponent);
	MechCapsule->SetCapsuleSize(400.0f, 2100.0f);
	MechCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MechCapsule->SetCollisionObjectType(ECC_Pawn);
	MechCapsule->SetCollisionResponseToAllChannels(ECR_Block);

	// Create procedural geometry
	ProceduralGeometry = CreateDefaultSubobject<UProceduralMechGeometry>(TEXT("ProceduralGeometry"));

	// Create systems
	MechMovement = CreateDefaultSubobject<UMechMovementComponent>(TEXT("MechMovement"));
	ReactorSystem = CreateDefaultSubobject<UReactorSystemComponent>(TEXT("ReactorSystem"));
	WeaponSystem = CreateDefaultSubobject<UWeaponSystemComponent>(TEXT("WeaponSystem"));
	DamageManagement = CreateDefaultSubobject<UDamageManagementComponent>(TEXT("DamageManagement"));
}

void AEnemyMechPawn::BeginPlay()
{
	Super::BeginPlay();

	// Bind to damage events
	// (Events will be checked in Tick instead)

	UE_LOG(LogTemp, Warning, TEXT("🤖 Enemy Mech spawned at %s"), *GetActorLocation().ToString());
	SetAIState(EAIState::Idle);
}

void AEnemyMechPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWasDestroyed)
		return;

	// Update AI
	UpdateAI(DeltaTime);

	// Update timers
	TimeSinceStateChange += DeltaTime;
	TimeSinceLastFire += DeltaTime;
	TimeSinceLastScan += DeltaTime;
}

float AEnemyMechPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageManagement)
	{
		// Apply damage to random section
		int32 RandomSection = FMath::RandRange(0, 5);
		EMechSection Section = static_cast<EMechSection>(RandomSection);

		DamageManagement->ApplyDamage(Section, ActualDamage, EDamageType::Ballistic, GetActorLocation());

		// If taking damage and not already engaged, switch to alert
		if (CurrentAIState == EAIState::Idle || CurrentAIState == EAIState::Patrol)
		{
			SetAIState(EAIState::Alert);

			// If damage causer exists, set as target
			if (DamageCauser)
			{
				CurrentTarget = DamageCauser;
				OnTargetAcquired.Broadcast(CurrentTarget);
			}
		}
	}

	return ActualDamage;
}

void AEnemyMechPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyMechPawn, CurrentAIState);
}

// ============================================================
// AI Behavior
// ============================================================

void AEnemyMechPawn::UpdateAI(float DeltaTime)
{
	// Execute current state behavior
	switch (CurrentAIState)
	{
	case EAIState::Idle:
		AI_Idle(DeltaTime);
		break;

	case EAIState::Patrol:
		AI_Patrol(DeltaTime);
		break;

	case EAIState::Alert:
		AI_Alert(DeltaTime);
		break;

	case EAIState::Engage:
		AI_Engage(DeltaTime);
		break;

	case EAIState::Retreat:
		AI_Retreat(DeltaTime);
		break;

	case EAIState::Disabled:
		AI_Disabled(DeltaTime);
		break;
	}
}

void AEnemyMechPawn::SetAIState(EAIState NewState)
{
	if (CurrentAIState == NewState)
		return;

	EAIState OldState = CurrentAIState;
	CurrentAIState = NewState;
	TimeSinceStateChange = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("🤖 Enemy AI state: %s → %s"),
	       *UEnum::GetValueAsString(OldState),
	       *UEnum::GetValueAsString(NewState));

	OnAIStateChanged.Broadcast(OldState, NewState);
}

AActor* AEnemyMechPawn::FindNearestTarget()
{
	AActor* NearestTarget = nullptr;
	float NearestDistance = DetectionRange;

	// Find all player pawns
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		// Skip self
		if (Actor == this)
			continue;

		// Skip non-player pawns
		APawn* Pawn = Cast<APawn>(Actor);
		if (!Pawn || !Pawn->IsPlayerControlled())
			continue;

		// Check distance
		float Distance = GetDistanceToTarget(Actor);
		if (Distance < NearestDistance)
		{
			// Check line of sight
			if (CanSeeTarget(Actor))
			{
				NearestDistance = Distance;
				NearestTarget = Actor;
			}
		}
	}

	return NearestTarget;
}

bool AEnemyMechPawn::CanSeeTarget(AActor* Target) const
{
	if (!Target)
		return false;

	FVector Start = GetActorLocation() + FVector(0, 0, 1000); // Eye height
	FVector End = Target->GetActorLocation() + FVector(0, 0, 1000);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	// If hit something, check if it's the target
	if (bHit)
	{
		return HitResult.GetActor() == Target;
	}

	// No hit means clear line of sight
	return true;
}

float AEnemyMechPawn::GetDistanceToTarget(AActor* Target) const
{
	if (!Target)
		return FLT_MAX;

	return FVector::Dist(GetActorLocation(), Target->GetActorLocation());
}

// ============================================================
// AI State Implementations
// ============================================================

void AEnemyMechPawn::AI_Idle(float DeltaTime)
{
	// Periodically scan for targets
	ScanForTargets(DeltaTime);

	// After some time, switch to patrol
	if (TimeSinceStateChange > 5.0f)
	{
		SetAIState(EAIState::Patrol);
	}
}

void AEnemyMechPawn::AI_Patrol(float DeltaTime)
{
	// Scan for targets
	ScanForTargets(DeltaTime);

	// Simple patrol: rotate slowly
	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw += 10.0f * DeltaTime;
	SetActorRotation(CurrentRotation);
}

void AEnemyMechPawn::AI_Alert(float DeltaTime)
{
	// Scan for targets more frequently
	TimeSinceLastScan += DeltaTime;
	if (TimeSinceLastScan >= ScanInterval * 0.5f)
	{
		TimeSinceLastScan = 0.0f;

		AActor* Target = FindNearestTarget();
		if (Target)
		{
			CurrentTarget = Target;
			SetAIState(EAIState::Engage);
			OnTargetAcquired.Broadcast(CurrentTarget);
		}
		else if (TimeSinceStateChange > 10.0f)
		{
			// No target found after 10 seconds, return to patrol
			SetAIState(EAIState::Patrol);
		}
	}

	// Face toward last known target location
	if (LastKnownTargetLocation != FVector::ZeroVector)
	{
		FVector DirectionToTarget = (LastKnownTargetLocation - GetActorLocation()).GetSafeNormal();
		FRotator TargetRotation = DirectionToTarget.Rotation();
		FRotator CurrentRotation = GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 2.0f);
		SetActorRotation(FRotator(0, NewRotation.Yaw, 0));
	}
}

void AEnemyMechPawn::AI_Engage(float DeltaTime)
{
	// Check if should retreat
	if (ShouldRetreat())
	{
		SetAIState(EAIState::Retreat);
		return;
	}

	// Verify target still valid
	if (!CurrentTarget || !CanSeeTarget(CurrentTarget))
	{
		LastKnownTargetLocation = CurrentTarget ? CurrentTarget->GetActorLocation() : FVector::ZeroVector;
		CurrentTarget = nullptr;
		SetAIState(EAIState::Alert);
		OnTargetLost.Broadcast();
		return;
	}

	// Update last known location
	LastKnownTargetLocation = CurrentTarget->GetActorLocation();

	// Move to optimal range
	MoveToOptimalRange(CurrentTarget, DeltaTime);

	// Aim at target
	AimAtTarget(CurrentTarget, DeltaTime);

	// Fire when ready
	if (TimeSinceLastFire >= FireInterval)
	{
		FireAtTarget(CurrentTarget);
		TimeSinceLastFire = 0.0f;
	}
}

void AEnemyMechPawn::AI_Retreat(float DeltaTime)
{
	// Move away from target
	if (CurrentTarget)
	{
		FVector DirectionAway = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
		FVector MoveDirection = DirectionAway;

		if (MechMovement)
		{
			MechMovement->ProcessMoveInput(FVector2D(MoveDirection.X, MoveDirection.Y));
		}

		// Still face target while retreating
		AimAtTarget(CurrentTarget, DeltaTime);
	}

	// If health recovered enough, re-engage
	if (DamageManagement && DamageManagement->GetTotalIntegrityPercent() > RetreatHealthThreshold + 0.1f)
	{
		SetAIState(EAIState::Engage);
	}
}

void AEnemyMechPawn::AI_Disabled(float DeltaTime)
{
	// Mech destroyed - do nothing
	if (!bWasDestroyed)
	{
		bWasDestroyed = true;
		OnMechDestroyed.Broadcast();
		UE_LOG(LogTemp, Error, TEXT("💀 Enemy Mech destroyed"));

		// Disable all systems
		if (MechMovement)
			MechMovement->SetActive(false);
		if (WeaponSystem)
			WeaponSystem->SetActive(false);
	}
}

// ============================================================
// Combat Actions
// ============================================================

void AEnemyMechPawn::AimAtTarget(AActor* Target, float DeltaTime)
{
	if (!Target)
		return;

	UpdateRotationTowardTarget(Target, DeltaTime);
}

void AEnemyMechPawn::FireAtTarget(AActor* Target)
{
	if (!Target || !WeaponSystem)
		return;

	// Check if facing target (within cone)
	FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector ForwardVector = GetActorForwardVector();
	float DotProduct = FVector::DotProduct(ForwardVector, DirectionToTarget);

	// Only fire if roughly facing target (within 30 degrees)
	if (DotProduct > 0.866f) // cos(30°)
	{
		// Apply accuracy modifier (random spread)
		float Spread = (1.0f - AccuracyModifier) * 100.0f; // Max 100cm spread at low accuracy

		WeaponSystem->FireWeapon();
		UE_LOG(LogTemp, Log, TEXT("🔫 Enemy fired at target"));
	}
}

void AEnemyMechPawn::MoveToOptimalRange(AActor* Target, float DeltaTime)
{
	if (!Target || !MechMovement)
		return;

	float Distance = GetDistanceToTarget(Target);

	if (Distance > OptimalCombatRange * 1.2f)
	{
		// Too far - move closer
		FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		MechMovement->ProcessMoveInput(FVector2D(DirectionToTarget.X, DirectionToTarget.Y));
	}
	else if (Distance < OptimalCombatRange * 0.8f)
	{
		// Too close - back up
		FVector DirectionAway = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
		MechMovement->ProcessMoveInput(FVector2D(DirectionAway.X, DirectionAway.Y) * -0.5f);
	}
	else
	{
		// At good range - stop moving
		MechMovement->ProcessMoveInput(FVector2D::ZeroVector);
	}
}

bool AEnemyMechPawn::ShouldRetreat() const
{
	if (!DamageManagement)
		return false;

	float HealthPercent = DamageManagement->GetTotalIntegrityPercent();
	return HealthPercent < RetreatHealthThreshold;
}

// ============================================================
// Internal Functions
// ============================================================

void AEnemyMechPawn::ScanForTargets(float DeltaTime)
{
	TimeSinceLastScan += DeltaTime;

	if (TimeSinceLastScan >= ScanInterval)
	{
		TimeSinceLastScan = 0.0f;

		AActor* Target = FindNearestTarget();
		if (Target)
		{
			CurrentTarget = Target;
			SetAIState(EAIState::Engage);
			OnTargetAcquired.Broadcast(CurrentTarget);
		}
	}
}

void AEnemyMechPawn::UpdateRotationTowardTarget(AActor* Target, float DeltaTime)
{
	if (!Target)
		return;

	FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FRotator TargetRotation = DirectionToTarget.Rotation();
	FRotator CurrentRotation = GetActorRotation();

	// Smooth rotation
	float RotationSpeed = 2.0f; // Slower than player for challenge
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);

	SetActorRotation(FRotator(0, NewRotation.Yaw, 0));
}

bool AEnemyMechPawn::IsAtOptimalRange(AActor* Target) const
{
	float Distance = GetDistanceToTarget(Target);
	return FMath::IsWithinInclusive(Distance, OptimalCombatRange * 0.8f, OptimalCombatRange * 1.2f);
}

bool AEnemyMechPawn::IsTooClose(AActor* Target) const
{
	float Distance = GetDistanceToTarget(Target);
	return Distance < MinimumRange;
}
