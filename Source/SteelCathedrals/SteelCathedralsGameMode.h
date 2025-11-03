// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SteelCathedralsGameMode.generated.h"

/**
 * Steel Cathedrals Game Mode
 * Handles spawning crew in mech, level setup
 * Supports 3-player multiplayer
 */
UCLASS()
class STEELCATHEDRALS_API ASteelCathedralsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASteelCathedralsGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void StartPlay() override;

public:
	// ============================================================
	// Spawn Configuration
	// ============================================================

	/** Where to spawn the Mech HULL in the world */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Mech")
	FVector MechSpawnLocation = FVector::ZeroVector;

	/** Mech spawn rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Mech")
	FRotator MechSpawnRotation = FRotator::ZeroRotator;

	/** Auto-capture screenshots on startup (for automated testing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
	bool bAutoCaptureScreenshots = false;

	/** Spawn locations for 3 crew members (RELATIVE to mech origin) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Crew")
	TArray<FVector> CrewSpawnLocations;

	/** Spawn rotations for 3 crew members (RELATIVE to mech rotation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Crew")
	TArray<FRotator> CrewSpawnRotations;

private:
	/** Map setup actor spawned to configure lighting/environment */
	UPROPERTY()
	class AMapSetupActor* MapSetup;

	/** The Mech HULL actor spawned in the world */
	UPROPERTY()
	class AMech* MechActor;

	/** Track how many players have spawned */
	int32 SpawnedPlayerCount = 0;

	/** Initialize default spawn locations */
	void InitializeSpawnLocations();

	/** Get spawn transform for next player */
	FTransform GetNextCrewSpawnTransform();
};
