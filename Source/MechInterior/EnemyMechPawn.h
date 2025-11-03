// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnemyMechPawn.generated.h"

class UMechMovementComponent;
class UReactorSystemComponent;
class UWeaponSystemComponent;
class UDamageManagementComponent;
class UProceduralMechGeometry;
class UCapsuleComponent;
class APlayerMechPawn;

/**
 * AI behavior states
 */
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Patrol UMETA(DisplayName = "Patrol"),
	Alert UMETA(DisplayName = "Alert"),
	Engage UMETA(DisplayName = "Engage"),
	Retreat UMETA(DisplayName = "Retreat"),
	Disabled UMETA(DisplayName = "Disabled")
};

/**
 * Enemy AI-controlled mech
 * Basic combat AI for testing player systems
 */
UCLASS()
class MECHINTERIOR_API AEnemyMechPawn : public APawn
{
	GENERATED_BODY()

public:
	AEnemyMechPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// ============================================================
	// Components
	// ============================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* MechRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* MechCapsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UMechMovementComponent* MechMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UReactorSystemComponent* ReactorSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponSystemComponent* WeaponSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDamageManagementComponent* DamageManagement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProceduralMechGeometry* ProceduralGeometry;

	// ============================================================
	// AI Configuration
	// ============================================================

	/** Detection range (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float DetectionRange = 500000.0f; // 5km

	/** Optimal combat range (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float OptimalCombatRange = 100000.0f; // 1km

	/** Minimum safe range (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float MinimumRange = 50000.0f; // 500m

	/** Retreat health threshold (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float RetreatHealthThreshold = 0.25f;

	/** Time between firing (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float FireInterval = 2.0f;

	/** Patrol idle time at waypoints (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float PatrolIdleTime = 3.0f;

	/** Accuracy modifier (0-1, higher = more accurate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	float AccuracyModifier = 0.7f;

	// ============================================================
	// AI State
	// ============================================================

	/** Current AI state */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI|State")
	EAIState CurrentAIState = EAIState::Idle;

	/** Current target (usually player mech) */
	UPROPERTY(BlueprintReadOnly, Category = "AI|State")
	AActor* CurrentTarget = nullptr;

	/** Time since last state change */
	UPROPERTY(BlueprintReadOnly, Category = "AI|State")
	float TimeSinceStateChange = 0.0f;

	/** Time since last fire */
	UPROPERTY(BlueprintReadOnly, Category = "AI|State")
	float TimeSinceLastFire = 0.0f;

	// ============================================================
	// AI Behavior
	// ============================================================

	/** Update AI behavior */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateAI(float DeltaTime);

	/** Change AI state */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAIState(EAIState NewState);

	/** Find nearest target */
	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* FindNearestTarget();

	/** Can see target (line of sight check) */
	UFUNCTION(BlueprintPure, Category = "AI")
	bool CanSeeTarget(AActor* Target) const;

	/** Get distance to target */
	UFUNCTION(BlueprintPure, Category = "AI")
	float GetDistanceToTarget(AActor* Target) const;

	// ============================================================
	// AI Actions
	// ============================================================

	/** Execute idle behavior */
	void AI_Idle(float DeltaTime);

	/** Execute patrol behavior */
	void AI_Patrol(float DeltaTime);

	/** Execute alert behavior (investigating) */
	void AI_Alert(float DeltaTime);

	/** Execute engage behavior (combat) */
	void AI_Engage(float DeltaTime);

	/** Execute retreat behavior */
	void AI_Retreat(float DeltaTime);

	/** Execute disabled behavior (destroyed) */
	void AI_Disabled(float DeltaTime);

	// ============================================================
	// Combat Actions
	// ============================================================

	/** Aim at target */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void AimAtTarget(AActor* Target, float DeltaTime);

	/** Fire at target */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void FireAtTarget(AActor* Target);

	/** Move to optimal range */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void MoveToOptimalRange(AActor* Target, float DeltaTime);

	/** Check if should retreat */
	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	bool ShouldRetreat() const;

	// ============================================================
	// Events
	// ============================================================

	/** Called when AI state changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIStateChanged, EAIState, OldState, EAIState, NewState);
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIStateChanged OnAIStateChanged;

	/** Called when target acquired */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetAcquired, AActor*, Target);
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnTargetAcquired OnTargetAcquired;

	/** Called when target lost */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetLost);
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnTargetLost OnTargetLost;

	/** Called when mech destroyed */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMechDestroyed);
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnMechDestroyed OnMechDestroyed;

private:
	// ============================================================
	// Internal Functions
	// ============================================================

	/** Scan for targets */
	void ScanForTargets(float DeltaTime);

	/** Update rotation toward target */
	void UpdateRotationTowardTarget(AActor* Target, float DeltaTime);

	/** Check if at optimal range */
	bool IsAtOptimalRange(AActor* Target) const;

	/** Check if too close */
	bool IsTooClose(AActor* Target) const;

	// ============================================================
	// State Tracking
	// ============================================================

	/** Time since last target scan */
	float TimeSinceLastScan = 0.0f;

	/** Scan interval */
	float ScanInterval = 1.0f;

	/** Last known target location */
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	/** Was destroyed */
	bool bWasDestroyed = false;
};
